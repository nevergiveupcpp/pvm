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

#ifndef NGU_PVM_TESTS_ASSETS_CRYPTO_H
#define NGU_PVM_TESTS_ASSETS_CRYPTO_H

#include <cstdint>
#include <utility>
#include <tuple>
#include <array>

namespace ngu::pvm::crypto {
    constexpr std::uint32_t XTEA_DELTA{ 0x9E3779B9 };
    constexpr std::uint32_t XTEA_ROUNDS{ 32 };
    constexpr std::uint32_t SUM_INIT{ XTEA_DELTA * XTEA_ROUNDS };

    namespace detail {
        struct rc4_state {
            std::array<std::uint8_t, 256> S;
            std::uint8_t i;
            std::uint8_t j;
        };

        constexpr std::uint32_t rotl32(std::uint32_t x, int n) {
            return (x << n) | (x >> (32 - n));
        }

        constexpr std::uint64_t rotl64(std::uint64_t x, int n) {
            return (x << n) | (x >> (64 - n));
        }

        constexpr rc4_state rc4_init(const std::uint8_t* key, std::size_t keylen) {
            rc4_state state{};

            for (int i{}; i < 256; i++) {
                state.S[i] = static_cast<std::uint8_t>(i);
            }

            int j{};
            for (int i{}; i < 256; i++) {
                j = (j + state.S[i] + key[i % keylen]) & 0xFF;
                std::uint8_t tmp{ state.S[i] };
                state.S[i] = state.S[j];
                state.S[j] = tmp;
            }

            state.i = 0;
            state.j = 0;
            return state;
        }

        constexpr std::uint8_t rc4_next_byte(rc4_state& state) {
            state.i = (state.i + 1) & 0xFF;
            state.j = (state.j + state.S[state.i]) & 0xFF;

            std::uint8_t tmp{ state.S[state.i] };
            state.S[state.i] = state.S[state.j];
            state.S[state.j] = tmp;

            return state.S[(state.S[state.i] + state.S[state.j]) & 0xFF];
        }
    }

    constexpr std::pair<std::uint32_t, std::uint32_t> xtea_encrypt(std::uint32_t v0, std::uint32_t v1, const std::uint32_t key[4]) {
        std::uint32_t sum{};
        for (int i{}; i < static_cast<int>(XTEA_ROUNDS); i++) {
            v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
            sum += XTEA_DELTA;
            v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        }
        return {v0, v1};
    }

    constexpr std::pair<std::uint32_t, std::uint32_t> xtea_decrypt(std::uint32_t v0, std::uint32_t v1, const std::uint32_t key[4]) {
        std::uint32_t sum{ XTEA_DELTA * XTEA_ROUNDS };
        for (int i{}; i < static_cast<int>(XTEA_ROUNDS); i++) {
            v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
            sum -= XTEA_DELTA;
            v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        }
        return {v0, v1};
    }

    constexpr std::uint8_t rc4_keystream_byte(const std::uint8_t* key, std::size_t keylen) {
        auto state = detail::rc4_init(key, keylen);
        return detail::rc4_next_byte(state);
    }

    constexpr std::uint8_t rc4_process(const std::uint8_t* key, std::size_t keylen, std::uint8_t data) {
        return data ^ rc4_keystream_byte(key, keylen);
    }

    constexpr void chacha20_quarter_round(std::uint32_t& a, std::uint32_t& b, std::uint32_t& c, std::uint32_t& d) {
        a += b; d ^= a; d = detail::rotl32(d, 16);
        c += d; b ^= c; b = detail::rotl32(b, 12);
        a += b; d ^= a; d = detail::rotl32(d, 8);
        c += d; b ^= c; b = detail::rotl32(b, 7);
    }

    constexpr std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t> chacha20_qr(
        std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d
    ) {
        chacha20_quarter_round(a, b, c, d);
        return {a, b, c, d};
    }
}

#endif //NGU_PVM_TESTS_ASSETS_CRYPTO_H
