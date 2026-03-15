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

#ifndef NGU_PVM_CODEGEN_CODEGEN_H
#define NGU_PVM_CODEGEN_CODEGEN_H

#include "assembler.h"

#include "../bytecode/bytecode.h"
#include "../bytecode/bytecode_decoder.h"

namespace ngu::pvm {
    namespace detail {
        /// @brief Returns the number of instructions in type T (1 for a single instruction, T::count for macro-instructions).
        template <typename T> consteval std::size_t instruction_count() {
            using U = std::remove_cvref_t<T>;
            if constexpr (requires { U::count; })
                return U::count;
            else
                return 1;
        }

        /// @brief Shrinks the bytecode to its actual size, removing the unused tail of the buffer.
        template<auto Gen> consteval auto shrink_to_fit() {
            constexpr auto big = Gen();
            constexpr std::size_t exact = big.length > 0 ? big.length : 1;

            std::uint8_t arr[exact]{};
            for (std::size_t i{}; i < big.length; ++i)
                arr[i] = big.bytes[i];
            return bytecode<exact>{ arr, big.length };
        }

        /// @brief First assembly pass: encodes instructions into bytecode with meta-opcodes and labels left unresolved.
        template<typename ... Instructions> consteval auto intermediate_assemble(Instructions&&... insns) {
            constexpr std::size_t count = (instruction_count<Instructions>() + ...);
            constexpr std::size_t max_size = count * 12;

            insn_data arr[count];
            std::size_t idx{};

            auto add = [&](auto&& arg) {
                using U = std::remove_cvref_t<decltype(arg)>;
                if constexpr (requires { U::count; }) {
                    for (std::size_t i{}; i < U::count; ++i)
                        arr[idx++] = arg.data[i];
                } else {
                    arr[idx++] = static_cast<insn_data>(arg);
                }
            };

            (add(std::forward<Instructions>(insns)), ...);

            std::uint8_t bytes[max_size]{};
            std::size_t pos{};

            for (std::size_t i{}; i < count; ++i) {
                auto const& insn = arr[i];
                auto total_size = get_total_size(insn.insn_sz);
                auto imm_size = get_imm_size(insn.insn_sz);
                auto hdr_size = total_size - imm_size;

                for (std::size_t j = 0; j < hdr_size; ++j)
                    bytes[pos++] = (insn.header >> (j * 8)) & 0xFF;
                for (std::size_t j = 0; j < imm_size; ++j)
                    bytes[pos++] = (insn.immediate >> (j * 8)) & 0xFF;
            }

            return bytecode<max_size>{ bytes, pos };
        }

        /// @brief Looks up the instruction index for a given label ID in the label table.
        consteval std::uint64_t find_label_target(const label_table& labels, std::uint64_t label_id) {
            for (std::size_t i{}; i < labels.size(); ++i)
                if (labels[i].label_id == label_id)
                    return labels[i].target_index;
            return 0;
        }

        /// @brief Second assembly pass: resolves labels and meta-opcodes, replacing them with real opcodes and jump targets.
        template<std::size_t N> consteval auto compile_bytecode(
            const bytecode<N>& code,
            const architecture& arch
        ) {
            std::uint8_t result[N]{};
            std::size_t result_pos{};

            auto decoder = make_bytecode_decoder<COMPILE_TIME>(code);

            auto insn_stream = decoder.get_instruction_stream();
            auto labels = decoder.get_label_table();

            for (std::size_t i{}; i < insn_stream.size(); ++i) {
                auto const& entry = insn_stream[i];

                if (entry.data_ct.is_meta && entry.data_ct.opcode == arch::meta_op::OP_LABEL) {
                    continue;
                }

                if (entry.data_ct.is_meta) {
                    auto const real_opcode = resolve_meta(
                        arch.ops, static_cast<arch::meta_op>(entry.data_ct.opcode)
                    );
                    auto const target = find_label_target(labels, entry.data_ct.immediate);

                    auto header = insn_decoder::read_header(&code.bytes[entry.offset]);

                    header = insn_encoder::patch_opcode(header, real_opcode);
                    insn_encoder::write_header(header, &result[result_pos]);

                    result_pos += sizeof(header);
                    auto const imm_size = entry.data_ct.insn_size - sizeof(header);

                    for (std::size_t j{}; j < imm_size; ++j) {
                        result[result_pos++] = (target >> (j * 8)) & 0xFF;
                    }

                } else {
                    for (std::size_t j{}; j < entry.data_ct.insn_size; ++j) {
                        result[result_pos++] = code.bytes[entry.offset + j];
                    }
                }
            }

            return bytecode<N>{ result, result_pos };
        }
    }

    /**
     * @brief Assembles bytecode from a set of instructions in two passes.
     *
     * The first pass (@ref detail::intermediate_assemble) encodes instructions with meta-opcodes.
     * The second pass (@ref detail::compile_bytecode) resolves labels and substitutes real opcodes.
     *
     * @param arch Architecture defining physical opcodes and registers.
     * @param insns Instructions to assemble.
     * @return Bytecode ready for execution.
     */
    template<typename ... Instructions> consteval auto assemble(const architecture& arch, Instructions&&... insns) {
        auto intermediate = detail::intermediate_assemble(std::forward<Instructions>(insns)...);
        return detail::compile_bytecode<decltype(intermediate)::capacity()>(intermediate, arch);
    }
}

// Assembles and shrinks bytecode to its exact size at compile time.
// Uses a captureless consteval lambda as a non-type template parameter to satisfy consteval context requirements.
#define PVM_ASSEMBLE(arch, ...) \
    ::ngu::pvm::detail::shrink_to_fit<[]() consteval { return ::ngu::pvm::assemble(arch, __VA_ARGS__); }>()

#endif //NGU_PVM_CODEGEN_CODEGEN_H