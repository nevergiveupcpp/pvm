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

#ifndef NGU_PVM_ENGINE_INTERPRETER_H
#define NGU_PVM_ENGINE_INTERPRETER_H

#include <cstdint>

#include "context.h"
#include "architecture.h"
#include "instructions.h"

#include "../bytecode/bytecode.h"
#include "../bytecode/bytecode_decoder.h"

namespace ngu::pvm {
    /**
     * @brief Executes bytecode against a given @ref architecture.
     *
     * The constructor is marked @c consteval and takes the architecture by value,
     * initializing the internal @ref context with the same copy.
     *
     * @par Fields
     * @c arch_ - copy of the @ref architecture object; used to resolve physical opcodes in @ref
     * dispatch(). @n
     * @c ctx_  - execution state; marked @c mutable since @ref run() is const but modifies the
     * context.
     *
     * @par Execution
     * Main execution loop - @ref run(); instruction dispatching - @ref dispatch().
     */
    class interpreter {
    public:
        /// @brief Execution completion code returned by @ref run() and @ref dispatch().
        enum class status : std::uint8_t {
            VM_SUCCESS,                   ///< Bytecode executed successfully or halted via @c HALT.
            VM_ERROR_UNKNOWN_OPCODE,      ///< Physical opcode does not match any known instruction.
            VM_ERROR_INVALID_REGISTER,    ///< Register index is out of bounds.
            VM_ERROR_PC_OUT_OF_BOUNDS,    ///< PC jumped outside the bytecode buffer.
            VM_ERROR_INVALID_INSTRUCTION, ///< Instruction header failed validation.
            VM_ERROR_HALTED,              ///< Execution attempted on an already halted context.
        };

        explicit consteval interpreter(const architecture& arch) : arch_(arch), ctx_(arch_) {
        }

        /**
         * @brief Executes bytecode @p cb until completion or an error.
         *
         * On each instruction, a context snapshot (@c ctx_ct) is created and written back
         * only on successful execution - ensuring atomicity of each step. @n
         * PC advances either via an explicit jump or by the size of the current instruction.
         *
         * @param cb Bytecode to execute.
         * @return @ref status completion code.
         */
        template<std::size_t N> PVM_FORCEINLINE status run(const bytecode<N>& cb) const;

        /// @brief Returns a copy of the architecture. @return @ref architecture by value.
        architecture get_arch() const {
            return arch_;
        }

        /// @brief Returns a pointer to the mutable execution context. @return Pointer to @ref
        /// context.
        context* get_ctx() const {
            return &ctx_;
        }

    private:
        PVM_FORCEINLINE status dispatch(context* ctx, const insn_view& insn, const insn_stream_rt& insn_entries) const;

        architecture arch_;
        mutable context ctx_;
    };

    template<std::size_t N> interpreter::status interpreter::run(const bytecode<N>& cb) const {
        const std::size_t size = cb.size();
        const std::uint8_t* pc = cb.bytes;
        const std::uint8_t* end = cb.bytes + size;

        auto decoder = make_bytecode_decoder<RUNTIME>(cb);

        while (pc < end && !ctx_.is_halted()) {
            if (pc >= end) {
                return status::VM_ERROR_PC_OUT_OF_BOUNDS;
            }

            auto ctx_ct = ctx_;
            ctx_ct.set_pc(static_cast<std::uint64_t>(pc - cb.bytes));

            auto const insn = insn_view(reinterpret_cast<const std::uint32_t*>(pc));

            auto const prev_pc = ctx_ct.get_pc();

            if (const auto status = dispatch(&ctx_ct, insn, decoder.get_instruction_stream());
                status != status::VM_SUCCESS) {
                return status;
            }

            if (ctx_ct.is_halted()) {
                return status::VM_SUCCESS;
            }

            ctx_ = ctx_ct;

            if (auto const curr_pc = ctx_ct.get_pc(); curr_pc != prev_pc) {
                if (curr_pc >= size) {
                    return status::VM_ERROR_PC_OUT_OF_BOUNDS;
                }
                pc = cb.bytes + curr_pc;
            } else {
                pc += insn.size();
            }
        }
        return status::VM_SUCCESS;
    }

    inline interpreter::status interpreter::dispatch(
        context* ctx, const insn_view& insn, const insn_stream_rt& insn_entries
    ) const {
        if (!insn.valid()) {
            return status::VM_ERROR_INVALID_INSTRUCTION;
        }

        // We use if/else for one simple reason: switch requires an integral constant expression,
        // but since arch_ is a class field, it is accessed through a runtime object (this), which
        // by definition cannot be a constant expression. Building a { opcode - dispatcher } table
        // would likely introduce overhead for 21 opcodes and negatively affect performance. The
        // decision regarding a table-based approach may be revisited in the future after further
        // benchmarking.

        if (auto const opc = insn.opcode(); opc == arch_.ops.MOV) {
            insn_dispatch<mov_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.XOR) {
            insn_dispatch<xor_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.ADD) {
            insn_dispatch<add_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.SUB) {
            insn_dispatch<sub_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.SHL) {
            insn_dispatch<shl_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.SHR) {
            insn_dispatch<shr_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.AND) {
            insn_dispatch<and_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.OR) {
            insn_dispatch<or_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.NOT) {
            insn_dispatch<not_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.ROL) {
            insn_dispatch<rol_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.ROR) {
            insn_dispatch<ror_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.CMP) {
            insn_dispatch<cmp_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.JMP) {
            insn_dispatch<jmp_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.JMPA) {
            insn_dispatch<jmpa_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.JMPI) {
            insn_dispatch<jmpi_base>::execute(ctx, insn, insn_entries);
        } else if (opc == arch_.ops.JNE) {
            insn_dispatch<jne_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.JNEI) {
            insn_dispatch<jnei_base>::execute(ctx, insn, insn_entries);
        } else if (opc == arch_.ops.JE) {
            insn_dispatch<je_base>::execute(ctx, insn);
        } else if (opc == arch_.ops.JEI) {
            insn_dispatch<jei_base>::execute(ctx, insn, insn_entries);
        } else if (opc == arch_.ops.NOP) {
            insn_dispatch<nop_base>::execute();
        } else if (opc == arch_.ops.HALT) {
            insn_dispatch<halt_base>::execute(ctx);
            return status::VM_SUCCESS;
        } else {
            return status::VM_ERROR_UNKNOWN_OPCODE;
        }
        return status::VM_SUCCESS;
    }
} // namespace ngu::pvm

#endif // NGU_PVM_ENGINE_INTERPRETER_H
