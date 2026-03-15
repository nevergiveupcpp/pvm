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

#ifndef NGU_PVM_CODEGEN_ASSEMBLER_H
#define NGU_PVM_CODEGEN_ASSEMBLER_H

#include <cstdint>

#include "codegen_types.h"

#include "../engine/architecture.h"
#include "../bytecode/instruction_encoder.h"

namespace ngu::pvm {
    /**
     * @brief Provides a compile-time interface for encoding bytecode instructions.
     *
     * The constructor takes @ref architecture by value (copy) to use physical opcode
     * and register indices when encoding instructions.
     * Each method returns @c detail::insn_data - a single encoded instruction.
     *
     * @par Instruction groups
     * Standard (@c dst + @c src):  @c MOV, @c ADD, @c SUB, @c XOR, @c AND, @c OR, @c SHL, @c SHR, @c ROL, @c ROR, @c CMP. @n
     * Unary (@c dst):              @c NOT. @n
     * Branches (@c offset):        @c JMP, @c JMPA, @c JMPI, @c JNE, @c JE, @c JNEI, @c JEI. @n
     * Meta (@c label_id):          @c LABEL, @c JMPL, @c JEL, @c JNEL - resolved into real instructions on the final pass. @n
     * No operands:                 @c NOP, @c HALT.
     *
     * @par Operand
     * The @c operand type accepts both a register (@c arch::reg) and an immediate value -
     * the addressing mode is selected automatically.
     */
    class assembler {
    public:
        consteval explicit assembler(const architecture &arch) : arch_(arch) {}

        consteval detail::insn_data NOT(const arch::reg dst) const {
            return {insn_encoder::make_header(arch_.ops.NOT, arch::insn_size::WORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), 0), 0, arch::insn_size::WORD};
        }

        consteval detail::insn_data MOV(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.MOV, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.MOV, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data XOR(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.XOR, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.XOR, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data ADD(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.ADD, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.ADD, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data SUB(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.SUB, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.SUB, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data SHL(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.SHL, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.SHL, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data SHR(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.SHR, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.SHR, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data AND(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.AND, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.AND, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data OR(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.OR, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.OR, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data ROL(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.ROL, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.ROL, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data ROR(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.ROR, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.ROR, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data CMP(const arch::reg dst, const operand src) const {
            if (src.is_imm) {
                auto const sz = detail::calc_insn_size(src.value, src.is_signed);
                return {insn_encoder::make_header(arch_.ops.CMP, sz, arch::insn_mode::IMMEDIATE, detail::phys_reg(arch_, dst), 0), src.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.CMP, arch::insn_size::DWORD, arch::insn_mode::REGISTER, detail::phys_reg(arch_, dst), detail::phys_reg(arch_, static_cast<arch::reg>(src.value))), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JMP(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JMP, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JMP, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JMPA(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JMPA, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JMPA, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JMPI(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JMPI, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JMPI, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JNE(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JNE, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JNE, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JE(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JE, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JE, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JNEI(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JNEI, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JNEI, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data JEI(const operand offset) const {
            if (offset.is_imm) {
                auto const sz = detail::calc_insn_size(offset.value, offset.is_signed);
                return {insn_encoder::make_header(arch_.ops.JEI, sz, arch::insn_mode::IMMEDIATE, 0, 0), offset.value, sz};
            }
            return {insn_encoder::make_header(arch_.ops.JEI, arch::insn_size::DWORD, arch::insn_mode::REGISTER, 0, static_cast<std::uint8_t>(offset.value)), 0, arch::insn_size::DWORD};
        }

        consteval detail::insn_data LABEL(const std::uint32_t label_id) const {
            return {insn_encoder::make_header(arch::meta_op::OP_LABEL, arch::insn_size::DWORD_D, arch::insn_mode::IMMEDIATE, 0, 0), label_id, arch::insn_size::DWORD_D};
        }

        consteval detail::insn_data JMPL(const std::uint32_t label_id) const {
            return {insn_encoder::make_header(arch::meta_op::OP_JMPL, arch::insn_size::DWORD_D, arch::insn_mode::IMMEDIATE, 0, 0), label_id, arch::insn_size::DWORD_D};
        }

        consteval detail::insn_data JEL(const std::uint32_t label_id) const {
            return {insn_encoder::make_header(arch::meta_op::OP_JEL, arch::insn_size::DWORD_D, arch::insn_mode::IMMEDIATE, 0, 0), label_id, arch::insn_size::DWORD_D};
        }

        consteval detail::insn_data JNEL(const std::uint32_t label_id) const {
            return {insn_encoder::make_header(arch::meta_op::OP_JNEL, arch::insn_size::DWORD_D, arch::insn_mode::IMMEDIATE, 0, 0), label_id, arch::insn_size::DWORD_D};
        }

        consteval detail::insn_data NOP() const {
            return {insn_encoder::make_header(arch_.ops.NOP, arch::insn_size::BYTE, arch::insn_mode::REGISTER, 0, 0), 0, arch::insn_size::BYTE};
        }

        consteval detail::insn_data HALT() const {
            return {insn_encoder::make_header(arch_.ops.HALT, arch::insn_size::BYTE, arch::insn_mode::REGISTER, 0, 0), 0, arch::insn_size::BYTE};
        }

    private:
        architecture arch_;
    };
}

#endif //NGU_PVM_CODEGEN_ASSEMBLER_H