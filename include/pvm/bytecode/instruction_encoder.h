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

#ifndef NGU_PVM_BYTECODE_INSTRUCTION_ENCODER_H
#define NGU_PVM_BYTECODE_INSTRUCTION_ENCODER_H

#include "../engine/architecture.h"

namespace ngu::pvm {
    /**
     * @brief Static utility set for encoding fields into a 32-bit instruction header.пше
     *
     * Symmetric to @ref instruction_decoder - performs the inverse operation:
     * packs fields into a header and writes it into a byte buffer.
     */
    struct instruction_encoder {
        using bits = arch::insn_bits;

        /// @brief Assembles a 32-bit header from instruction fields.
        static constexpr std::uint32_t make_header(
            std::uint8_t opcode, arch::insn_size insn_sz, arch::insn_mode mode, std::uint8_t dst, std::uint8_t src
        ) {
            return static_cast<std::uint32_t>(opcode) | (static_cast<std::uint32_t>(insn_sz) << bits::INSN_SIZE) |
                   (static_cast<std::uint32_t>(mode) << bits::MODE) |
                   (static_cast<std::uint32_t>(dst) << bits::DESTINATION) |
                   (static_cast<std::uint32_t>(src) << bits::SOURCE);
        }

        /// @brief Writes @p header into @p data (little-endian, 4 bytes).
        static constexpr void write_header(std::uint32_t header, std::uint8_t* data) {
            data[0] = header & 0xFF;
            data[1] = (header >> 8) & 0xFF;
            data[2] = (header >> 16) & 0xFF;
            data[3] = (header >> 24) & 0xFF;
        }

        /// @brief Replaces the opcode in an existing header with @p opcode, leaving all other
        /// fields intact.
        static constexpr std::uint32_t patch_opcode(std::uint32_t header, std::uint8_t opcode) {
            return (header & ~0xFFu) | opcode;
        }

        /// @brief Writes @p value into @p data (little-endian) according to @p insn_sz. Returns the
        /// number of bytes written.
        static constexpr std::size_t encode_immediate(
            std::uint64_t value, arch::insn_size insn_sz, std::uint8_t* data
        ) {
            auto const cnt = detail::get_imm_size(insn_sz);
            for (std::size_t i{}; i < cnt; ++i) {
                data[i] = static_cast<std::uint8_t>(value >> (i * 8));
            }
            return cnt;
        }
    };

    using insn_encoder = instruction_encoder;
} // namespace ngu::pvm

#endif // NGU_PVM_BYTECODE_INSTRUCTION_ENCODER_H