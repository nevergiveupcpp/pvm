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

#ifndef NGU_PVM_BYTECODE_BYTECODE_H
#define NGU_PVM_BYTECODE_BYTECODE_H

#include <cstdint>
#include <utility>

namespace ngu::pvm {
    namespace detail {
        // Computes the byte contribution of a single argument to the total bytecode capacity:
        // - integral type   → 1 byte
        // - bytecode<N>     → N bytes (via ::capacity())
        // - raw array T[N]  → N bytes
        template<typename T> consteval std::size_t contribution() {
            using U = std::remove_cvref_t<T>;
            if constexpr (std::is_integral_v<U>) {
                return 1;
            } else if constexpr (requires { U::capacity(); }) {
                return U::capacity();
            } else if constexpr (std::is_array_v<U>) {
                return std::extent_v<U>;
            }
            return 0;
        }
    } // namespace detail

    /**
     * @brief Fixed-capacity compile-time byte buffer holding @p N bytes of bytecode.
     *
     * @tparam N Buffer capacity in bytes. Deduced automatically via CTAD deduction guide.
     *
     * @par Constructors
     * Variadic constructor - accepts any combination of: @n
     * - @c std::uint8_t   - single byte; @n
     * - @c bytecode<M>    - another bytecode buffer (contributes M bytes); @n
     * - raw array @c T[M] - contributes M bytes. @n
     * The deduction guide computes @p N as the sum of all argument contributions at compile time.
     * @n Array constructor - copies the first @c len bytes from a @c uint8_t[N] array.
     *
     * @par Fields
     * @c bytes   - raw byte storage. @n
     * @c length  - number of bytes written; never exceeds @p N.
     */
    template<std::size_t N> struct bytecode {
        template<typename... Args> consteval explicit bytecode(Args&&... args) {
            (append(std::forward<Args>(args)), ...);
        }

        consteval bytecode(std::uint8_t const (&arr)[N], std::size_t len) {
            for (std::size_t i = 0; i < len && i < N; ++i) {
                bytes[length++] = arr[i];
            }
        }

        /// @brief Returns the buffer capacity in bytes (@c N).
        static constexpr std::size_t capacity() {
            return N;
        }

        /// @brief Returns the number of bytes written so far.
        constexpr std::size_t size() const {
            return length;
        }

        std::uint8_t bytes[N]{};
        std::size_t length{};

    private:
        consteval void append(std::uint8_t const (&arr)[N]) {
            for (std::size_t i = 0; i < N && length < N; ++i) {
                bytes[length++] = arr[i];
            }
        }

        template<std::size_t M> consteval void append(std::uint8_t const (&arr)[M]) {
            for (std::size_t i = 0; i < M && length < N; ++i) {
                bytes[length++] = arr[i];
            }
        }

        consteval void append(std::uint8_t byte) {
            if (length < N) {
                bytes[length++] = byte;
            }
        }
    };

    template<typename... Args> bytecode(Args&&... args) -> bytecode<(detail::contribution<Args>() + ...)>;
} // namespace ngu::pvm

#endif // NGU_PVM_BYTECODE_BYTECODE_H