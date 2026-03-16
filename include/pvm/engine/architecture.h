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

#ifndef NGU_PVM_ENGINE_ARCHITECTURE_H
#define NGU_PVM_ENGINE_ARCHITECTURE_H

#include <cstdint>
#include <limits>
#include <type_traits>

#include "../utilities/platform.h"
#include "../utilities/utilities.h"

namespace ngu::pvm {
    namespace detail {
        consteval std::uint64_t lcg_next(const std::uint64_t state) {
            return state * 6364136223846793005ULL + 1442695040888963407ULL;
        }

        consteval void shuffle_indices(std::uint8_t* arr, const std::size_t count, const std::uint64_t entropy) {
            for (std::size_t i{}; i < count; i++) {
                arr[i] = static_cast<std::uint8_t>(i);
            }

            std::uint64_t rng_state{entropy};
            for (std::size_t i{count - 1}; i > 0; i--) {
                rng_state = lcg_next(rng_state);
                std::size_t j{rng_state % (i + 1)};
                std::uint8_t tmp{arr[i]};
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    } // namespace detail

    /**
     * @brief Describes the ISA (Instruction Set Architecture) and ABI (Application Binary
     * Interface).
     *
     * The entire architecture is evaluated at compile time (consteval).
     *
     * @par Instruction Set (ISA)
     * Instruction format:       @c OPCODE(0) | INSN_SIZE(8) | MODE(11) | DESTINATION(12) |
     * SOURCE(16) @n Without immediate:        @c BYTE(2B), @c WORD(2B), @c DWORD(4B) @n With
     * immediate:           @c DWORD_B(5/1B), @c DWORD_W(6/2B), @c DWORD_D(8/4B), @c DWORD_Q(12/8B)
     * @n Addressing modes:         @c REGISTER, @c IMMEDIATE @n Registers:                @c R0–R11
     * - 12 general-purpose registers; @c FLAGS and @c PC - special-purpose. @n Meta-opcodes: @c
     * OP_LABEL, @c OP_JMPL, @c OP_JEL, @c OP_JNEL - markers resolved into real branch instructions
     * on the final pass. @n Standard opcodes:         21 opcodes describing the VM execution logic
     * (@c MOV, @c ADD, @c SUB, ...).
     *
     * @par Binary Interface (ABI)
     * Physical indices of registers and opcodes are randomized independently and stored
     * in @c registers::physical_map and @c opcodes::physical_map mapping tables.
     * Initialization is performed via @ref make().
     */
    class architecture {
    public:
        template<bool HasImmediate = false> struct instruction {

            enum class mode : std::uint8_t { REGISTER, IMMEDIATE };

            enum class insn_size : std::uint8_t {
                BYTE,
                WORD,
                DWORD,
                DWORD_B,
                DWORD_W,
                DWORD_D,
                DWORD_Q,
            };

            struct bits {
                static constexpr std::uint8_t OPCODE = 0;
                static constexpr std::uint8_t INSN_SIZE = 8;
                static constexpr std::uint8_t MODE = 11;
                static constexpr std::uint8_t DESTINATION = 12;
                static constexpr std::uint8_t SOURCE = 16;
            };

            union {
                struct {
                    std::uint32_t op : 8;
                    std::uint32_t size : 3;
                    std::uint32_t mod : 1;
                    std::uint32_t src : 4;
                    std::uint32_t dst : 4;
                    std::uint32_t reserved_ : 12;
                };
                std::uint32_t data{};
            };

            [[no_unique_address]]
            std::conditional_t<HasImmediate, std::uint64_t, empty> immediate{};
        };

        using insn_bits = instruction<>::bits;
        using insn_mode = instruction<>::mode;
        using insn_size = instruction<>::insn_size;

        struct registers {
            enum logical_reg : std::uint8_t {
                REG_R0,
                REG_R1,
                REG_R2,
                REG_R3,
                REG_R4,
                REG_R5,
                REG_R6,
                REG_R7,
                REG_R8,
                REG_R9,
                REG_R10,
                REG_R11,
                REG_FLAGS,
                REG_PC,
                REG_MAX
            };

            union {
                struct {
                    struct {
                        std::uint8_t R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11;
                    } gpr;

                    struct {
                        std::uint8_t FLAGS, PC;
                    } spr;
                };
                std::uint8_t data[REG_MAX]{};
            };

            mapping_entry<logical_reg> physical_map[REG_MAX];
        };
        registers regs{};

        using reg = registers::logical_reg;

        struct opcodes {
            enum meta_op : std::uint8_t {
                OP_META_START = 0xE0,
                OP_LABEL = OP_META_START,
                OP_JMPL,
                OP_JEL,
                OP_JNEL,
            };

            enum logical_op : std::uint8_t {
                OP_MOV,
                OP_ADD,
                OP_SUB,
                OP_AND,
                OP_OR,
                OP_XOR,
                OP_NOT,
                OP_SHL,
                OP_SHR,
                OP_ROL,
                OP_ROR,
                OP_CMP,
                OP_JMP,
                OP_JMPA,
                OP_JMPI,
                OP_JNE,
                OP_JE,
                OP_JNEI,
                OP_JEI,
                OP_NOP,
                OP_HALT,
                OP_MAX,
            };

            static_assert(
                static_cast<std::uint8_t>(OP_MAX) < static_cast<std::uint8_t>(OP_META_START), "Opcodes out of bounds"
            );

            union {
                struct {
                    std::uint8_t MOV;
                    std::uint8_t ADD, SUB;
                    std::uint8_t AND, OR, XOR, NOT;
                    std::uint8_t SHL, SHR, ROL, ROR;
                    std::uint8_t CMP;
                    std::uint8_t JMP, JMPA, JMPI;
                    std::uint8_t JNE, JE, JNEI, JEI;
                    std::uint8_t NOP, HALT;
                };
                std::uint8_t data[OP_MAX]{};
            };

            mapping_entry<logical_op> physical_map[OP_MAX];

            static constexpr mapping_entry<meta_op, logical_op> meta_map[] = {
                {OP_JMPL, OP_JMPI},
                {OP_JEL, OP_JEI},
                {OP_JNEL, OP_JNEI},
            };
        };
        opcodes ops{};

        using op = opcodes::logical_op;
        using meta_op = opcodes::meta_op;

        /**
         * @brief Generates the ABI of the virtual machine.
         *
         * Determines the physical representation of registers and opcodes,
         * and builds their logical-to-physical mapping tables.
         * Indices are randomized via Fisher-Yates shuffle seeded with @p entropy (LCG-based).
         * Registers and opcodes are shuffled independently: opcodes use @c lcg_next(entropy) as
         * seed.
         *
         * @param entropy Seed value. The same seed always produces the same ABI.
         * @return Initialized @ref architecture instance.
         */
        static consteval architecture make(std::uint64_t entropy);
    };

    consteval architecture architecture::make(const std::uint64_t entropy) {
        architecture arch;

        std::uint8_t reg_perm[reg::REG_MAX];
        detail::shuffle_indices(reg_perm, reg::REG_MAX, entropy);

        std::uint8_t op_perm[op::OP_MAX];
        detail::shuffle_indices(op_perm, op::OP_MAX, detail::lcg_next(entropy));

#define INIT_REG(path, idx) arch.regs.path = reg_perm[idx]

        INIT_REG(gpr.R0, reg::REG_R0);
        INIT_REG(gpr.R1, reg::REG_R1);
        INIT_REG(gpr.R2, reg::REG_R2);
        INIT_REG(gpr.R3, reg::REG_R3);
        INIT_REG(gpr.R4, reg::REG_R4);
        INIT_REG(gpr.R5, reg::REG_R5);
        INIT_REG(gpr.R6, reg::REG_R6);
        INIT_REG(gpr.R7, reg::REG_R7);
        INIT_REG(gpr.R8, reg::REG_R8);
        INIT_REG(gpr.R9, reg::REG_R9);
        INIT_REG(gpr.R10, reg::REG_R10);
        INIT_REG(gpr.R11, reg::REG_R11);

        INIT_REG(spr.FLAGS, reg::REG_FLAGS);
        INIT_REG(spr.PC, reg::REG_PC);

#undef INIT_REG

        for (std::size_t i{}; i < reg::REG_MAX; ++i) {
            arch.regs.physical_map[i] = mapping_entry(static_cast<reg>(i), reg_perm[i]);
        }

#define INIT_OP(name, idx) arch.ops.name = op_perm[idx]

        INIT_OP(MOV, op::OP_MOV);
        INIT_OP(ADD, op::OP_ADD);
        INIT_OP(SUB, op::OP_SUB);
        INIT_OP(AND, op::OP_AND);
        INIT_OP(OR, op::OP_OR);
        INIT_OP(XOR, op::OP_XOR);
        INIT_OP(NOT, op::OP_NOT);
        INIT_OP(SHL, op::OP_SHL);
        INIT_OP(SHR, op::OP_SHR);
        INIT_OP(ROL, op::OP_ROL);
        INIT_OP(ROR, op::OP_ROR);
        INIT_OP(CMP, op::OP_CMP);
        INIT_OP(JMP, op::OP_JMP);
        INIT_OP(JMPA, op::OP_JMPA);
        INIT_OP(JMPI, op::OP_JMPI);
        INIT_OP(JNE, op::OP_JNE);
        INIT_OP(JE, op::OP_JE);
        INIT_OP(JNEI, op::OP_JNEI);
        INIT_OP(JEI, op::OP_JEI);
        INIT_OP(NOP, op::OP_NOP);
        INIT_OP(HALT, op::OP_HALT);

#undef INIT_OP

        for (std::size_t i{}; i < op::OP_MAX; ++i) {
            arch.ops.physical_map[i] = mapping_entry(static_cast<op>(i), op_perm[i]);
        }

        return arch;
    }

    using arch = architecture;

    namespace detail {
        PVM_FORCEINLINE constexpr std::uint8_t phys_reg(const architecture& arch, const arch::reg r) {
            return (r < arch::reg::REG_MAX) ? arch.regs.physical_map[r].to : 0;
        }

        PVM_FORCEINLINE constexpr std::uint8_t resolve_meta(const arch::opcodes& ops, arch::meta_op m) {
            for (const auto& [from, to] : arch::opcodes::meta_map) {
                if (from == m) {
                    return ops.physical_map[to].to;
                }
            }
            return 0;
        }

        PVM_FORCEINLINE constexpr arch::insn_size calc_insn_size(
            const std::uint64_t val, const bool is_signed = false
        ) {
            if (is_signed) {
                auto const signed_val = static_cast<std::int64_t>(val);

                if (signed_val >= std::numeric_limits<std::int8_t>::min() &&
                    signed_val <= std::numeric_limits<std::int8_t>::max()) {
                    return arch::insn_size::DWORD_B;
                }
                if (signed_val >= std::numeric_limits<std::int16_t>::min() &&
                    signed_val <= std::numeric_limits<std::int16_t>::max()) {
                    return arch::insn_size::DWORD_W;
                }
                if (signed_val >= std::numeric_limits<std::int32_t>::min() &&
                    signed_val <= std::numeric_limits<std::int32_t>::max()) {
                    return arch::insn_size::DWORD_D;
                }
                return arch::insn_size::DWORD_Q;
            }

            if (val <= std::numeric_limits<std::uint8_t>::max()) {
                return arch::insn_size::DWORD_B;
            }
            if (val <= std::numeric_limits<std::uint16_t>::max()) {
                return arch::insn_size::DWORD_W;
            }
            if (val <= std::numeric_limits<std::uint32_t>::max()) {
                return arch::insn_size::DWORD_D;
            }
            return arch::insn_size::DWORD_Q;
        }

        PVM_FORCEINLINE constexpr std::size_t get_total_size(const arch::insn_size sz) {
            constexpr std::size_t sizes[]{2, 2, 4, 5, 6, 8, 12};
            return sizes[static_cast<std::uint8_t>(sz)];
        }

        PVM_FORCEINLINE constexpr std::size_t get_imm_size(const arch::insn_size sz) {
            constexpr std::size_t imm_sizes[]{0, 0, 0, 1, 2, 4, 8};
            return imm_sizes[static_cast<std::uint8_t>(sz)];
        }
    } // namespace detail
} // namespace ngu::pvm

#endif // NGU_PVM_ENGINE_ARCHITECTURE_H