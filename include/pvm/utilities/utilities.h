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

#ifndef NGU_PVM_UTILITIES_UTILITIES_H
#define NGU_PVM_UTILITIES_UTILITIES_H

#include <cstddef>

namespace ngu::pvm {
    /// @brief Zero-size empty struct. Used with @c [[no_unique_address]] to conditionally
    /// exclude fields from structures without any memory overhead.
    struct empty {};

    /**
     * @brief CRTP base providing indexed access to derived class data.
     *
     * The derived class must implement @c size_impl() and @c at_impl(i).
     * The base class adds @c size(), @c operator[] and @c for_each().
     *
     * @tparam Derived Concrete derived type (CRTP).
     */
    template<typename Derived> class indexed_view {
    public:
        constexpr std::size_t size() const {
            return self().size_impl();
        }

        constexpr decltype(auto) operator[](std::size_t i) const {
            return self().at_impl(i);
        }

        template<typename Fn> constexpr void for_each(Fn&& fn) const {
            for (std::size_t i{}; i < size(); ++i) {
                fn((*this)[i]);
            }
        }

    protected:
        constexpr const Derived& self() const {
            return static_cast<const Derived&>(*this);
        }
    };

    /**
     * @brief Stores a { from - to } pair mapping one value to another.
     *
     * @c from - source value. @n
     * @c to   - mapped value.
     *
     * @tparam From Type of the source value.
     * @tparam To   Type of the mapped value. Defaults to @c std::uint8_t.
     */
    template<class From, class To = std::uint8_t> struct mapping_entry {
        constexpr mapping_entry() = default;
        consteval mapping_entry(const From from, const To to) : from(from), to(to) {
        }

        From from{};
        To to{};
    };
} // namespace ngu::pvm

#endif // NGU_PVM_UTILITIES_UTILITIES_H
