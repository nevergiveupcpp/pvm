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

#ifndef NGU_PVM_ENGINE_INSTRUCTIONS_H
#define NGU_PVM_ENGINE_INSTRUCTIONS_H

#include "context.h"

#include "../utilities/platform.h"
#include "../bytecode/bytecode_decoder.h"

namespace ngu::pvm {
    /**
     * @brief Instruction dispatcher based on the Strategy pattern.
     *
     * @c execute() constructs a temporary @c Strategy object and calls @c impl() with the provided arguments.
     * The pattern allows swapping primitive implementations - for example,
     * obfuscated variants such as @c xored_mov, @c xored_add, etc. could be introduced in the future.
     *
     * @tparam Strategy Type implementing the @c impl() method with the required signature.
     */
    template<typename Strategy> struct instruction_dispatch {
        template<typename... Args> PVM_FORCEINLINE static void execute(Args... args) {
            Strategy{}.impl(std::forward<Args>(args)...);
        }
    };

    template<typename Strategy>
    using insn_dispatch = instruction_dispatch<Strategy>;

    struct mov_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), val);
        }
    };

    struct xor_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) ^ val);
        }
    };

    struct add_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) + val);
        }
    };

    struct sub_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) - val);
        }
    };

    struct shl_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const shift = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) << shift);
        }
    };

    struct shr_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const shift = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) >> shift);
        }
    };

    struct and_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) & val);
        }
    };

    struct or_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->set_reg(insn.destination(), ctx->get_reg(insn.destination()) | val);
        }
    };

    struct not_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            ctx->set_reg(insn.destination(), ~ctx->get_reg(insn.destination()));
        }
    };

    struct rol_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = ctx->get_reg(insn.destination());
            auto shift = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            shift &= 63;
            ctx->set_reg(insn.destination(), (val << shift) | (val >> (64 - shift)));
        }
    };

    struct ror_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const val = ctx->get_reg(insn.destination());
            auto shift = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            shift &= 63;
            ctx->set_reg(insn.destination(), (val >> shift) | (val << (64 - shift)));
        }
    };

    struct cmp_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const a = ctx->get_reg(insn.destination());
            auto const b = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());

            std::uint64_t flags{};
            if (a == b) flags |= (1ULL << 0);
            if (a < b)  flags |= (1ULL << 1);

            ctx->set_flags(flags);
        }
    };

    struct jmp_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const offset = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->jump(ctx->get_pc() + insn.size() + offset);
        }
    };

    struct jmpa_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            auto const offset = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
            ctx->jump(offset);
        }
    };

    struct jmpi_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn, const insn_stream_rt& insn_entries) {
            auto const pos = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());

            if (pos >= insn_entries.size()) {
                return;
            }

            ctx->jump(insn_entries[pos].offset);
        }
    };

    struct jne_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            if (!(ctx->get_flags() & 1)) {
                auto const offset = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
                ctx->jump(ctx->get_pc() + insn.size() + offset);
            }
        }
    };

    struct je_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn) {
            if (ctx->get_flags() & 1) {
                auto const offset = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());
                ctx->jump(ctx->get_pc() + insn.size() + offset);
            }
        }
    };

    struct jnei_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn, const insn_stream_rt& insn_entries) {
            if (!(ctx->get_flags() & 1)) {
                auto const pos = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());

                if (pos >= insn_entries.size()) {
                    return;
                }

                ctx->jump(insn_entries[pos].offset);
            }
        }
    };

    struct jei_base {
        PVM_FORCEINLINE static void impl(context *ctx, const insn_view& insn, const insn_stream_rt& insn_entries) {
            if (ctx->get_flags() & 1) {
                auto const pos = insn.has_immediate() ? insn.immediate() : ctx->get_reg(insn.source());

                if (pos >= insn_entries.size()) {
                    return;
                }

                ctx->jump(insn_entries[pos].offset);
            }
        }
    };

    struct halt_base {
        PVM_FORCEINLINE static void impl(context *ctx) {
            ctx->halt();
        }
    };

    struct nop_base {
        PVM_FORCEINLINE static void impl() {
            //NOP
        }
    };
}

#endif //NGU_PVM_ENGINE_INSTRUCTIONS_H
