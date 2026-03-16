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

#ifndef NGU_PVM_BYTECODE_INSTRUCTION_DECODER_H
#define NGU_PVM_BYTECODE_INSTRUCTION_DECODER_H

#include "../engine/architecture.h"

namespace ngu::pvm {
    /**
     * @brief Static utility set for extracting fields from a 32-bit instruction header.
     *
     * All methods accept a raw @c header and return the corresponding field
     * according to the format: @c OPCODE(0) | INSN_SIZE(8) | MODE(11) | DESTINATION(12) |
     * SOURCE(16).
     */
    struct instruction_decoder {
        using bits = arch::insn_bits;

        /// @brief Extracts the physical opcode (bits 0-7).
        static constexpr std::uint8_t opcode(const std::uint32_t header) {
            return (header >> bits::OPCODE) & 0xFF;
        }

        /// @brief Extracts the operand size index (bits 8-10), corresponds to @c arch::insn_size.
        static constexpr std::uint8_t insn_size(const std::uint32_t header) {
            return (header >> bits::INSN_SIZE) & 0x7;
        }

        /// @brief Extracts the addressing mode (bit 11): 0 - REGISTER, 1 - IMMEDIATE.
        static constexpr std::uint8_t mode(const std::uint32_t header) {
            return (header >> bits::MODE) & 0x1;
        }

        /// @brief Extracts the physical destination register index (bits 12-15).
        static constexpr std::uint8_t destination(const std::uint32_t header) {
            return (header >> bits::DESTINATION) & 0xF;
        }

        /// @brief Extracts the physical source register index (bits 16-19).
        static constexpr std::uint8_t source(const std::uint32_t header) {
            return (header >> bits::SOURCE) & 0xF;
        }

        /// @brief Returns true if the instruction uses immediate addressing mode.
        static constexpr bool has_immediate(const std::uint32_t header) {
            return mode(header) == static_cast<std::uint8_t>(arch::insn_mode::IMMEDIATE);
        }

        /// @brief Returns the total instruction size in bytes (header + immediate).
        static constexpr std::size_t total_size(const std::uint32_t header) {
            return detail::get_total_size(static_cast<arch::insn_size>(insn_size(header)));
        }

        /// @brief Reads the immediate value from @p data (little-endian) after the header. Returns
        /// 0 if no immediate is present.
        static constexpr std::uint64_t immediate(const std::uint32_t header, const std::uint8_t* data) {
            if (!has_immediate(header)) {
                return 0;
            }
            auto const sz = static_cast<arch::insn_size>(insn_size(header));
            auto const cnt = detail::get_imm_size(sz);

            std::uint64_t result{};
            for (std::size_t i{}; i < cnt; ++i) {
                result |= static_cast<std::uint64_t>(data[i]) << (i * 8);
            }
            return result;
        }

        /// @brief Reads 4 bytes from @p data and assembles them into a 32-bit header
        /// (little-endian).
        static constexpr std::uint32_t read_header(const std::uint8_t* data) {
            return static_cast<std::uint32_t>(data[0]) | (static_cast<std::uint32_t>(data[1]) << 8) |
                   (static_cast<std::uint32_t>(data[2]) << 16) | (static_cast<std::uint32_t>(data[3]) << 24);
        }
    };

    using insn_decoder = instruction_decoder;

    /**
     * @brief Non-owning view over a pointer to an instruction in the bytecode buffer.
     *
     * Wraps @ref instruction_decoder calls behind a convenient object interface.
     * @c valid() checks that the pointer is non-null. @c immediate() reads bytes immediately
     * after the 4-byte header (@c ip_ + 1).
     */
    class instruction_view {
    public:
        explicit instruction_view(const std::uint32_t* ptr = nullptr) : ip_(ptr) {
        }

        std::uint32_t data() const {
            return valid() ? *ip_ : 0;
        }

        std::uint8_t opcode() const {
            return insn_decoder::opcode(data());
        }

        std::uint8_t insn_size() const {
            return insn_decoder::insn_size(data());
        }

        std::uint8_t mode() const {
            return insn_decoder::mode(data());
        }

        std::uint8_t destination() const {
            return insn_decoder::destination(data());
        }

        std::uint8_t source() const {
            return insn_decoder::source(data());
        }

        bool has_immediate() const {
            return insn_decoder::has_immediate(data());
        }

        std::uint64_t immediate() const {
            return valid() ? insn_decoder::immediate(data(), reinterpret_cast<const std::uint8_t*>(ip_ + 1)) : 0;
        }

        std::size_t size() const {
            return insn_decoder::total_size(data());
        }

        bool valid() const {
            return ip_ != nullptr;
        }

    private:
        const std::uint32_t* ip_{};
    };

    using insn_view = instruction_view;
} // namespace ngu::pvm

#endif // NGU_PVM_BYTECODE_INSTRUCTION_DECODER_H