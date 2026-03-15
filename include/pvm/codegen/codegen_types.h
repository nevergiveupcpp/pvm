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

#ifndef NGU_PVM_CODEGEN_CODEGEN_TYPES_H
#define NGU_PVM_CODEGEN_CODEGEN_TYPES_H

#include <cstddef>

#include "../engine/architecture.h"

namespace ngu::pvm {
    namespace detail {
        /**
         * @brief A single encoded instruction produced by @ref assembler methods.
         *
         * @c header    - encoded 32-bit instruction word. @n
         * @c immediate - immediate value (0 if absent). @n
         * @c insn_sz   - instruction size variant; used by @c size() to compute the total byte length.
         */
        struct instruction_data {
            constexpr std::size_t size() const {
                return get_total_size(insn_sz);
            }

            std::uint32_t header{};
            std::uint64_t immediate{};
            arch::insn_size insn_sz{};
        };

        using insn_data = instruction_data;

        /**
         * @brief A fixed-size compile-time array of @ref instruction_data produced by @ref macro_assembler methods.
         *
         * @c count - number of instructions, deduced from the @c Types pack. @n
         * @c data  - array of encoded instructions.
         *
         * @tparam Types Pack of @ref instruction_data instances; its size determines @c count.
         */
        template <typename... Types> struct instruction_sequence {
            static constexpr std::size_t count = sizeof...(Types);

            consteval explicit instruction_sequence(Types&&... args)
                : data{ static_cast<instruction_data>(args)... } {}

            instruction_data data[sizeof...(Types)];
        };

        template <typename... Types>
        using insn_seq = instruction_sequence<Types...>;
    }

    /**
     * @brief Unified operand type accepted by @ref assembler and @ref macro_assembler methods.
     *
     * Constructed from an integral immediate value or an @c arch::reg register. @n
     * @c value     - register index or immediate value. @n
     * @c is_imm    - true if constructed from an integral value; false if from @c arch::reg. @n
     * @c is_signed - true if the integral type is signed; used to select the minimal instruction size.
     */
    struct operand {
        template<typename Type>
        explicit constexpr operand(const Type v) requires std::is_unsigned_v<Type> || std::is_integral_v<Type>
            : value(v), is_imm(true), is_signed(std::is_signed_v<Type>) {}
        constexpr explicit operand(const arch::reg r)
            : value(r) {}

        std::uint64_t value{};
        bool is_signed{};
        bool is_imm{};
    };
}

#endif //NGU_PVM_CODEGEN_CODEGEN_TYPES_H