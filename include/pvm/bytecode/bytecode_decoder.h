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

#ifndef NGU_PVM_BYTECODE_BYTECODE_DECODER_H
#define NGU_PVM_BYTECODE_BYTECODE_DECODER_H

#include <cstdint>

#include "bytecode.h"
#include "instruction_decoder.h"

#include "../engine/architecture.h"

namespace ngu::pvm {
    /// @brief Bytecode decoding mode.
    /// @c RUNTIME      - stores only instruction offsets; used by the interpreter at runtime. @n
    /// @c COMPILE_TIME - additionally stores opcodes, immediates and metadata; builds a label table.
    enum decode_time { RUNTIME, COMPILE_TIME };

    /**
     * @brief A single instruction entry in the decoder stream.
     *
     * @c offset  - instruction offset in the bytecode in bytes. @n
     * @c data_ct - extended data, present only when @c DecodeTime == COMPILE_TIME;
     *              takes 0 bytes in @c RUNTIME mode via @c [[no_unique_address]].
     *
     * @tparam DecodeTime Decoding mode.
     */
    template<decode_time DecodeTime> struct instruction_entry {
        std::uint64_t offset{};

        /**
         * @brief Extended instruction data available only in compile-time mode.
         *
         * @c opcode    - physical opcode of the instruction. @n
         * @c index     - sequential index of the instruction in the stream. @n
         * @c immediate - immediate value (0 if absent). @n
         * @c insn_size - total size of the instruction in bytes. @n
         * @c is_meta   - true if the instruction is a meta-opcode.
         */
        struct extension_data_ct {
            std::uint64_t opcode{};
            std::uint64_t index{};
            std::uint64_t immediate{};
            std::size_t insn_size{};
            bool is_meta{};
        };

        [[no_unique_address]]
        std::conditional_t<DecodeTime == COMPILE_TIME, extension_data_ct, empty> data_ct{};
    };

    template<decode_time DecodeTime>
    using insn_entry = instruction_entry<DecodeTime>;

    using insn_entry_rt = insn_entry<RUNTIME>;
    using insn_entry_ct = insn_entry<COMPILE_TIME>;

    /**
     * @brief A single entry in the label table.
     *
     * @c label_id     - label identifier (immediate value from @c OP_LABEL). @n
     * @c target_index - index of the next real instruction after the label (meta-instructions are not counted). @n
     * @c target       - pointer to the next instruction entry in the stream.
     */
    struct label_entry {
        std::uint64_t label_id;
        std::uint64_t target_index;
        insn_entry_ct* target;
    };

    /// @brief View over the decoder instruction stream. @tparam DecodeTime Decoding mode.
    template<decode_time DecodeTime> class instruction_stream :
        public indexed_view<instruction_stream<DecodeTime>> {
    public:
        using value_type = instruction_entry<DecodeTime>;

        constexpr instruction_stream(const value_type* data,
                                     const std::size_t count)
            : data_(data), count_(count) {}

        constexpr const value_type& at_impl(std::size_t i) const {
            return data_[i];
        }

        constexpr std::size_t size_impl() const {
            return count_;
        }

    private:
        const value_type* data_;
        std::size_t count_;
    };

    template<decode_time DecodeTime>
    using insn_stream = instruction_stream<DecodeTime>;

    using insn_stream_rt = insn_stream<RUNTIME>;
    using insn_stream_ct = insn_stream<COMPILE_TIME>;

    /// @brief View over the label table. Built only when @c DecodeTime == COMPILE_TIME.
    struct label_table : public indexed_view<label_table> {
        using value_type = label_entry;

        constexpr label_table(const value_type* data,
                              const std::size_t count)
            : data_(data), count_(count) {}

        constexpr const value_type& at_impl(std::size_t i) const {
            return data_[i];
        }

        constexpr std::size_t size_impl() const {
            return count_;
        }

    private:
        const value_type* data_;
        std::size_t count_;
    };

    /**
     * @brief Parses a @ref bytecode into an instruction stream and a label table.
     *
     * In @c RUNTIME mode only the instruction offset stream is built. @n
     * In @c COMPILE_TIME mode extended data for each instruction is also populated
     * and a label table is built to resolve @c OP_LABEL branches.
     *
     * @tparam DecodeTime Decoding mode (@c RUNTIME or @c COMPILE_TIME).
     * @tparam N          Bytecode capacity; maximum instruction count is @c (N+3)/4.
     */
    template<decode_time DecodeTime, std::size_t N> class bytecode_decoder {
        static constexpr std::size_t MAX_INSN = (N + 3) / 4; //(N + divisor - 1) / divisor
    public:
        constexpr explicit bytecode_decoder(const bytecode<N> &code);

        /// @brief Returns the instruction stream. @return @ref insn_stream for the given @c DecodeTime.
        constexpr insn_stream<DecodeTime> get_instruction_stream() const {
            return insn_stream{insns_, insn_count_};
        }

        /// @brief Returns the label table. Relevant only when @c DecodeTime == COMPILE_TIME.
        constexpr label_table get_label_table() const {
            return label_table{labels_, label_count_};
        }

        /// @brief Returns the bytecode capacity in bytes.
        static constexpr std::size_t capacity() {
            return N;
        }

        /// @brief Returns the maximum instruction count - @c (N+3)/4.
        static constexpr std::size_t insn_capacity() {
            return MAX_INSN;
        }

    private:
        // Iterates over bytecode bytes and fills insns_[]. In COMPILE_TIME mode also extracts opcodes, immediates and meta flags.
        constexpr void build_instruction_stream() {
            const std::uint8_t *pc{ code_.bytes };
            const std::uint8_t *end{ code_.bytes + code_.size() };
            std::size_t index{};

            while (pc < end && index < MAX_INSN) {
                auto const header = insn_decoder::read_header(pc);

                auto& entry = insns_[index];
                entry.offset = pc - code_.bytes;

                auto const total_size = insn_decoder::total_size(header);

                if constexpr (DecodeTime == COMPILE_TIME) {
                    entry.data_ct.opcode = insn_decoder::opcode(header);
                    entry.data_ct.insn_size = total_size;
                    entry.data_ct.index = index;
                    entry.data_ct.is_meta = (entry.data_ct.opcode >= arch::meta_op::OP_META_START);

                    entry.data_ct.immediate = insn_decoder::immediate(header, pc + 4);
                }

                pc += total_size;
                index++;
            }
            insn_count_ = index;
        }

        // Scans insns_[] for OP_LABEL entries and fills labels_[]. new_idx counts only real (non-meta) instructions.
        constexpr void build_label_table() {
            std::uint64_t new_idx{};
            for (std::size_t i{}; i < insn_count_; ++i) {
                auto const& entry = insns_[i];
                if (entry.data_ct.is_meta && entry.data_ct.opcode == arch::meta_op::OP_LABEL) {
                    labels_[label_count_].label_id = entry.data_ct.immediate;
                    labels_[label_count_].target = &insns_[i + 1];
                    labels_[label_count_].target_index = new_idx;
                    ++label_count_;
                }
                else {
                    ++new_idx;
                }
            }
        }

        const bytecode<N>& code_{};

        insn_entry<DecodeTime> insns_[MAX_INSN]{};
        std::size_t insn_count_{};

        label_entry labels_[MAX_INSN]{};
        std::size_t label_count_{};
    };

    /// @brief Factory function for @ref bytecode_decoder; deduces @c N automatically from @p code.
    template<decode_time DecodeTime, std::size_t N> constexpr auto make_bytecode_decoder(const bytecode<N>& code) {
        return bytecode_decoder<DecodeTime, N>(code);
    }

    template<decode_time DecodeTime, std::size_t N> constexpr bytecode_decoder<DecodeTime, N>::bytecode_decoder(const bytecode<N> &code) : code_(code) {
        build_instruction_stream();
        if constexpr (DecodeTime == COMPILE_TIME) {
            build_label_table();
        }
    }
}

#endif //NGU_PVM_BYTECODE_BYTECODE_DECODER_H