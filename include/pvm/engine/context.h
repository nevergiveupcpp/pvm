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

#ifndef NGU_PVM_ENGINE_CONTEXT_H
#define NGU_PVM_ENGINE_CONTEXT_H

#include <cstdint>

#include "architecture.h"

#include "../utilities/platform.h"

namespace ngu::pvm {
    /**
     * @brief Holds the bytecode execution state for a given @ref architecture.
     *
     * The constructor takes the architecture by value (copy) and is marked @c consteval,
     * allowing the context to be created at compile time.
     *
     * @par Fields
     * @c arch_    - copy of the @ref architecture object; defines physical indices of registers and opcodes. @n
     * @c regs_    - register value array, indexed by physical indices. @n
     * @c halted_  - execution stop flag; set via @ref halt().
     *
     * @par Methods
     * Each register operation is available in two variants: @n
     * - runtime (@c PVM_FORCEINLINE): @c set_reg, @c get_reg, @c set_flags, @c set_pc, @c jump, @c halt. @n
     * - compile-time (@c consteval):  @c set_reg_ct, @c set_flags_ct, @c set_pc_ct.
     */
    class context {
    public:
        explicit consteval context(const architecture& arch) :
            arch_(arch) {};

        /// @brief Writes @p val to register at physical index @p idx.
        PVM_FORCEINLINE void set_reg(const std::uint8_t idx, const std::uint64_t val) {
            regs_[idx] = val;
        }

        /// @brief Reads register value at physical index @p idx.
        PVM_FORCEINLINE std::uint64_t get_reg(const std::uint8_t idx) const {
            return regs_[idx];
        }

        /// @brief Writes @p val to logical register @p r, resolved to its physical index.
        PVM_FORCEINLINE void set_reg(const arch::reg r, const std::uint64_t val) {
            regs_[detail::phys_reg(arch_, r)] = val;
        }

        /// @brief Reads logical register @p r, resolved to its physical index.
        PVM_FORCEINLINE std::uint64_t get_reg(const arch::reg r) const {
            return regs_[detail::phys_reg(arch_, r)];
        }

        /// @brief Writes @p f to the FLAGS special-purpose register.
        PVM_FORCEINLINE void set_flags(const std::uint64_t f) {
            regs_[arch_.regs.spr.FLAGS] = f;
        }

        /// @brief Sets the PC register to @p offset.
        PVM_FORCEINLINE void set_pc(const std::uint64_t offset) {
            regs_[arch_.regs.spr.PC] = offset;
        }

        /// @brief Compile-time variant of @ref set_reg(uint8_t, uint64_t).
        consteval void set_reg_ct(const std::uint8_t idx, const std::uint64_t val) {
            regs_[idx] = val;
        }

        /// @brief Compile-time variant of @ref set_reg(arch::reg, uint64_t).
        consteval void set_reg_ct(const arch::reg r, const std::uint64_t val) {
            regs_[detail::phys_reg(arch_, r)] = val;
        }

        /// @brief Compile-time variant of @ref set_flags().
        consteval void set_flags_ct(const std::uint64_t f) {
            regs_[arch_.regs.spr.FLAGS] = f;
        }

        /// @brief Compile-time variant of @ref set_pc().
        consteval void set_pc_ct(const std::uint64_t offset) {
            regs_[arch_.regs.spr.PC] = offset;
        }

        /// @brief Returns the current value of the FLAGS register.
        PVM_FORCEINLINE std::uint64_t get_flags() const {
            return regs_[arch_.regs.spr.FLAGS];
        }

        /// @brief Returns the current value of the PC register.
        PVM_FORCEINLINE std::uint64_t get_pc() const {
            return regs_[arch_.regs.spr.PC];
        }

        /// @brief Sets PC to @p offset, effectively performing a jump.
        PVM_FORCEINLINE void jump(const std::uint64_t offset) {
            set_pc(offset);
        }

        /// @brief Sets the halted flag, stopping execution at the next cycle check.
        PVM_FORCEINLINE void halt() {
            halted_ = true;
        }

        /// @brief Returns true if the context has been halted.
        PVM_FORCEINLINE bool is_halted() const {
            return halted_;
        }

    private:
        architecture arch_;
        std::uint64_t regs_[arch::reg::REG_MAX]{};
        bool halted_{};
    };
}

#endif //NGU_PVM_ENGINE_CONTEXT_H