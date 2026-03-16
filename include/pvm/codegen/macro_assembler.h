// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 nevergiveupcpp

// Copyright 2025-2026 nevergiveupcpp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NGU_PVM_CODEGEN_MACRO_ASSEMBLER_H
#define NGU_PVM_CODEGEN_MACRO_ASSEMBLER_H

#include "assembler.h"
#include "codegen_types.h"

namespace ngu::pvm {
    /**
     * @brief Extends @ref assembler with composite compile-time macro-instructions.
     *
     * Unlike @ref assembler, each method returns @c detail::insn_seq -
     * a sequence of multiple instructions. Many macros accept a working register
     * @c scratch - it is used for intermediate computations and is clobbered upon completion.
     * Macros with branching also accept a @c label_id.
     *
     * @par Macro groups
     * Arithmetic:        @c ZERO, @c NEG, @c MOV64, @c MUL_POW2, @c DIV_POW2, @c MUL3, @c MUL5. @n
     * Comparison/flow:   @c ABS, @c MIN, @c MAX, @c LOOP. @n
     * Bit manipulation:  @c MASK_LO, @c CLEAR_LO, @c EXTRACT_BYTE, @c INSERT_BYTE, @c ROT16, @c
     * BSWAP32, @c TEST_BIT.
     * @n 64-bit ops:        @c INC64, @c ADD64. @n Crypto primitives: @c ARX, @c XR, @c AX, @c XS
     * (ChaCha/Salsa-style operations). @n Utilities:         @c MOVC, @c SWAP, @c ALIGN_UP.
     */
    class macro_assembler : public assembler {
    public:
        consteval explicit macro_assembler(const architecture& arch) : assembler(arch) {
        }

        // dst = 0
        consteval auto ZERO(const arch::reg dst) const {
            return detail::insn_seq{XOR(dst, operand(dst))};
        }

        // dst = -dst (two's complement)
        consteval auto NEG(const arch::reg dst) const {
            return detail::insn_seq{NOT(dst), ADD(dst, operand(1u))};
        }

        // dst = src (via xor+or)
        consteval auto MOVC(const arch::reg dst, const arch::reg src) const {
            return detail::insn_seq{XOR(dst, operand(dst)), OR(dst, operand(src))};
        }

        // dst = (hi << 32) | lo
        consteval auto MOV64(const arch::reg dst, std::uint32_t hi, std::uint32_t lo) const {
            return detail::insn_seq{MOV(dst, operand(hi)), SHL(dst, operand(32u)), OR(dst, operand(lo))};
        }

        // swap a, b without temp
        consteval auto SWAP(const arch::reg a, const arch::reg b) const {
            return detail::insn_seq{XOR(a, operand(b)), XOR(b, operand(a)), XOR(a, operand(b))};
        }

        // dst = |dst| (signed). scratch is clobbered.
        consteval auto ABS(const arch::reg dst, const arch::reg scratch, std::uint32_t label_id) const {
            return detail::insn_seq{
                MOV(scratch, operand(dst)),
                SHR(scratch, operand(63u)), // scratch = 1 if negative, 0 if positive/zero
                CMP(scratch, operand(0u)),
                JEL(label_id),              // if positive/zero, skip negation
                NOT(dst),
                ADD(dst, operand(1u)),
                LABEL(label_id)
            };
        }

        // a = min(a, b) unsigned. scratch is clobbered.
        consteval auto MIN(const arch::reg a, const arch::reg b, const arch::reg scratch, std::uint32_t label_id)
            const {
            return detail::insn_seq{
                CMP(a, operand(b)),                          // bit0=EQ, bit1=LT(a<b unsigned)
                JEL(label_id),                               // if a == b, keep a
                MOV(scratch, operand(arch::reg::REG_FLAGS)), // save flags before they're clobbered
                AND(scratch, operand(2u)),                   // isolate LT bit
                CMP(scratch, operand(0u)),
                JNEL(label_id),                              // if LT was set (a < b), keep a
                MOV(a, operand(b)),                          // a > b, so a = b
                LABEL(label_id)
            };
        }

        // a = max(a, b) unsigned. scratch is clobbered.
        consteval auto MAX(const arch::reg a, const arch::reg b, const arch::reg scratch, std::uint32_t label_id)
            const {
            return detail::insn_seq{
                CMP(a, operand(b)),                          // bit0=EQ, bit1=LT(a<b unsigned)
                JEL(label_id),                               // if a == b, keep a
                MOV(scratch, operand(arch::reg::REG_FLAGS)), // save flags before they're clobbered
                AND(scratch, operand(2u)),                   // isolate LT bit
                CMP(scratch, operand(0u)),
                JEL(label_id),                               // if LT was not set (a > b), keep a
                MOV(a, operand(b)),                          // a < b, so a = b
                LABEL(label_id)
            };
        }

        // dst *= 2^n
        consteval auto MUL_POW2(const arch::reg dst, std::uint8_t n) const {
            return detail::insn_seq{SHL(dst, operand(n))};
        }

        // dst /= 2^n (unsigned)
        consteval auto DIV_POW2(const arch::reg dst, std::uint8_t n) const {
            return detail::insn_seq{SHR(dst, operand(n))};
        }

        // dst *= 3
        consteval auto MUL3(const arch::reg dst, const arch::reg scratch) const {
            return detail::insn_seq{MOV(scratch, operand(dst)), SHL(scratch, operand(1u)), ADD(dst, operand(scratch))};
        }

        // dst *= 5
        consteval auto MUL5(const arch::reg dst, const arch::reg scratch) const {
            return detail::insn_seq{MOV(scratch, operand(dst)), SHL(scratch, operand(2u)), ADD(dst, operand(scratch))};
        }

        // dst = (dst + align-1) & ~(align-1), align must be power of 2
        consteval auto ALIGN_UP(const arch::reg dst, std::uint32_t align) const {
            return detail::insn_seq{ADD(dst, operand(align - 1)), AND(dst, operand(~(align - 1)))};
        }

        // (lo, hi) += 1 with carry
        consteval auto INC64(const arch::reg lo, const arch::reg hi, std::uint32_t label_id) const {
            return detail::insn_seq{
                ADD(lo, operand(1u)), CMP(lo, operand(0u)), JNEL(label_id), ADD(hi, operand(1u)), LABEL(label_id)
            };
        }

        // (dst_lo, dst_hi) += (src_lo, src_hi) with carry
        consteval auto ADD64(
            const arch::reg dst_lo,
            const arch::reg dst_hi,
            const arch::reg src_lo,
            const arch::reg src_hi,
            const arch::reg tmp,
            std::uint32_t label_id
        ) const {
            return detail::insn_seq{
                MOV(tmp, operand(dst_lo)),
                ADD(dst_lo, operand(src_lo)),
                CMP(dst_lo, operand(tmp)),
                JNEL(label_id),
                ADD(dst_hi, operand(1u)),
                LABEL(label_id),
                ADD(dst_hi, operand(src_hi))
            };
        }

        // flags = (src >> bit) & 1 == 0
        consteval auto TEST_BIT(const arch::reg src, const arch::reg scratch, std::uint8_t bit) const {
            return detail::insn_seq{
                MOV(scratch, operand(1u)),
                SHL(scratch, operand(bit)),
                AND(scratch, operand(src)),
                CMP(scratch, operand(0u))
            };
        }

        // dst &= (1 << n) - 1
        consteval auto MASK_LO(const arch::reg dst, std::uint8_t n) const {
            return detail::insn_seq{AND(dst, operand((1ull << n) - 1))};
        }

        // dst &= ~((1 << n) - 1)
        consteval auto CLEAR_LO(const arch::reg dst, std::uint8_t n) const {
            return detail::insn_seq{AND(dst, operand(~((1ull << n) - 1)))};
        }

        // scratch = (src >> idx*8) & 0xFF
        consteval auto EXTRACT_BYTE(const arch::reg src, const arch::reg scratch, std::uint8_t idx) const {
            return detail::insn_seq{
                MOV(scratch, operand(src)),
                SHR(scratch, operand(static_cast<std::uint8_t>(idx * 8u))),
                AND(scratch, operand(0xFFu))
            };
        }

        // dst |= (scratch & 0xFF) << idx*8
        consteval auto INSERT_BYTE(const arch::reg dst, const arch::reg scratch, std::uint8_t idx) const {
            return detail::insn_seq{
                AND(scratch, operand(0xFFu)),
                SHL(scratch, operand(static_cast<std::uint8_t>(idx * 8u))),
                OR(dst, operand(scratch))
            };
        }

        // dst = swap upper/lower 16 bits
        consteval auto ROT16(const arch::reg dst) const {
            return detail::insn_seq{ROL(dst, operand(16u))};
        }

        // dst = byte-reverse dst
        consteval auto BSWAP32(const arch::reg dst, const arch::reg s1, const arch::reg s2) const {
            return detail::insn_seq{
                MOV(s1, operand(dst)),
                AND(s1, operand(0xFFu)),
                SHL(s1, operand(24u)),
                MOV(s2, operand(dst)),
                AND(s2, operand(0xFF00u)),
                SHL(s2, operand(8u)),
                OR(s1, operand(s2)),
                MOV(s2, operand(dst)),
                SHR(s2, operand(8u)),
                AND(s2, operand(0xFF00u)),
                OR(s1, operand(s2)),
                MOV(s2, operand(dst)),
                SHR(s2, operand(24u)),
                AND(s2, operand(0xFFu)),
                OR(s1, operand(s2)),
                MOV(dst, operand(s1))
            };
        }

        // a += b; c ^= a; c <<<= n
        consteval auto ARX(const arch::reg a, const arch::reg b, const arch::reg c, std::uint8_t rot) const {
            return detail::insn_seq{ADD(a, operand(b)), XOR(c, operand(a)), ROL(c, operand(rot))};
        }

        // a ^= b; a <<<= n
        consteval auto XR(const arch::reg a, const arch::reg b, std::uint8_t rot) const {
            return detail::insn_seq{XOR(a, operand(b)), ROL(a, operand(rot))};
        }

        // a += b; c ^= a
        consteval auto AX(const arch::reg a, const arch::reg b, const arch::reg c) const {
            return detail::insn_seq{ADD(a, operand(b)), XOR(c, operand(a))};
        }

        // scratch = (v << left) ^ (v >> right)
        consteval auto XS(
            const arch::reg v, const arch::reg scratch, const arch::reg tmp, std::uint8_t left, std::uint8_t right
        ) const {
            return detail::insn_seq{
                MOV(scratch, operand(v)),
                SHL(scratch, operand(left)),
                MOV(tmp, operand(v)),
                SHR(tmp, operand(right)),
                XOR(scratch, operand(tmp))
            };
        }

        // counter -= 1; jump to body_label if counter != 0
        consteval auto LOOP(const arch::reg counter, std::uint32_t body_label) const {
            return detail::insn_seq{SUB(counter, operand(1u)), CMP(counter, operand(0u)), JNEL(body_label)};
        }
    };
} // namespace ngu::pvm

#endif // NGU_PVM_CODEGEN_MACRO_ASSEMBLER_H