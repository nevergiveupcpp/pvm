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

#ifndef NGU_PVM_TESTS_BENCHMARK_H
#define NGU_PVM_TESTS_BENCHMARK_H

#include <benchmark/benchmark.h>

#include "../assets/crypto.h"

#include "pvm/codegen/codegen.h"
#include "pvm/engine/interpreter.h"

using namespace ngu::pvm;

class VirtualMachineBenchmark : public benchmark::Fixture {
protected:
    static constexpr auto arch = architecture::make(1);
    static constexpr auto cr = assembler(arch);
};

BENCHMARK_F(VirtualMachineBenchmark, XteaDecrypt_Native)(benchmark::State& state) {
    static constexpr std::uint32_t KEY[4] = {0x00112233, 0x44556677, 0x8899AABB, 0xCCDDEEFF};
    static constexpr std::uint32_t PLAIN_V0 = 0x01234567;
    static constexpr std::uint32_t PLAIN_V1 = 0x89ABCDEF;
    static constexpr auto cipher = crypto::xtea_encrypt(PLAIN_V0, PLAIN_V1, KEY);

    for (auto _ : state) {
        auto [v0, v1] = crypto::xtea_decrypt(cipher.first, cipher.second, KEY);
        benchmark::DoNotOptimize(v0);
        benchmark::DoNotOptimize(v1);
    }
}

BENCHMARK_F(VirtualMachineBenchmark, XteaDecrypt_VM)(benchmark::State& state) {
    static constexpr std::uint32_t KEY[4] = {0x00112233, 0x44556677, 0x8899AABB, 0xCCDDEEFF};
    static constexpr std::uint32_t PLAIN_V0 = 0x01234567;
    static constexpr std::uint32_t PLAIN_V1 = 0x89ABCDEF;
    static constexpr auto cipher = crypto::xtea_encrypt(PLAIN_V0, PLAIN_V1, KEY);

    static constexpr std::uint64_t LOOP_START = 1;
    static constexpr std::uint64_t SWITCH1_KEY1 = 2;
    static constexpr std::uint64_t SWITCH1_KEY2 = 3;
    static constexpr std::uint64_t SWITCH1_KEY3 = 4;
    static constexpr std::uint64_t SWITCH1_END = 5;
    static constexpr std::uint64_t SWITCH2_KEY1 = 6;
    static constexpr std::uint64_t SWITCH2_KEY2 = 7;
    static constexpr std::uint64_t SWITCH2_KEY3 = 8;
    static constexpr std::uint64_t SWITCH2_END = 9;

    // R1=v0(cipher), R2=v1(cipher), R3=sum, R4=delta, R5=rounds_counter
    // R6/R7/R8 = scratch
    static constexpr auto code = PVM_ASSEMBLE(
        arch,
        cr.MOV(arch::reg::REG_R1, operand(cipher.first)),
        cr.MOV(arch::reg::REG_R2, operand(cipher.second)),
        cr.MOV(arch::reg::REG_R3, operand(crypto::SUM_INIT)),
        cr.MOV(arch::reg::REG_R4, operand(crypto::XTEA_DELTA)),
        cr.MOV(arch::reg::REG_R5, operand(crypto::XTEA_ROUNDS)),

        cr.LABEL(LOOP_START),

        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R1)),
        cr.SHL(arch::reg::REG_R6, operand(4u)),
        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R1)),
        cr.SHR(arch::reg::REG_R7, operand(5u)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R7)),
        cr.ADD(arch::reg::REG_R6, operand(arch::reg::REG_R1)),

        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R3)),
        cr.SHR(arch::reg::REG_R7, operand(11u)),
        cr.AND(arch::reg::REG_R7, operand(3u)),

        cr.CMP(arch::reg::REG_R7, operand(0u)),
        cr.JNEL(SWITCH1_KEY1),
        cr.MOV(arch::reg::REG_R8, operand(KEY[0])),
        cr.JMPL(SWITCH1_END),

        cr.LABEL(SWITCH1_KEY1),
        cr.CMP(arch::reg::REG_R7, operand(1u)),
        cr.JNEL(SWITCH1_KEY2),
        cr.MOV(arch::reg::REG_R8, operand(KEY[1])),
        cr.JMPL(SWITCH1_END),

        cr.LABEL(SWITCH1_KEY2),
        cr.CMP(arch::reg::REG_R7, operand(2u)),
        cr.JNEL(SWITCH1_KEY3),
        cr.MOV(arch::reg::REG_R8, operand(KEY[2])),
        cr.JMPL(SWITCH1_END),

        cr.LABEL(SWITCH1_KEY3),
        cr.MOV(arch::reg::REG_R8, operand(KEY[3])),

        cr.LABEL(SWITCH1_END),

        cr.ADD(arch::reg::REG_R8, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R8)),
        cr.SUB(arch::reg::REG_R2, operand(arch::reg::REG_R6)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),

        cr.SUB(arch::reg::REG_R3, operand(arch::reg::REG_R4)),
        cr.AND(arch::reg::REG_R3, operand(0xFFFFFFFFu)),

        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R2)),
        cr.SHL(arch::reg::REG_R6, operand(4u)),
        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R2)),
        cr.SHR(arch::reg::REG_R7, operand(5u)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R7)),
        cr.ADD(arch::reg::REG_R6, operand(arch::reg::REG_R2)),

        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R3)),
        cr.AND(arch::reg::REG_R7, operand(3u)),

        cr.CMP(arch::reg::REG_R7, operand(0u)),
        cr.JNEL(SWITCH2_KEY1),
        cr.MOV(arch::reg::REG_R8, operand(KEY[0])),
        cr.JMPL(SWITCH2_END),

        cr.LABEL(SWITCH2_KEY1),
        cr.CMP(arch::reg::REG_R7, operand(1u)),
        cr.JNEL(SWITCH2_KEY2),
        cr.MOV(arch::reg::REG_R8, operand(KEY[1])),
        cr.JMPL(SWITCH2_END),

        cr.LABEL(SWITCH2_KEY2),
        cr.CMP(arch::reg::REG_R7, operand(2u)),
        cr.JNEL(SWITCH2_KEY3),
        cr.MOV(arch::reg::REG_R8, operand(KEY[2])),
        cr.JMPL(SWITCH2_END),

        cr.LABEL(SWITCH2_KEY3),
        cr.MOV(arch::reg::REG_R8, operand(KEY[3])),

        cr.LABEL(SWITCH2_END),

        cr.ADD(arch::reg::REG_R8, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R8)),
        cr.SUB(arch::reg::REG_R1, operand(arch::reg::REG_R6)),
        cr.AND(arch::reg::REG_R1, operand(0xFFFFFFFFu)),

        cr.SUB(arch::reg::REG_R5, operand(1u)),
        cr.CMP(arch::reg::REG_R5, operand(0u)),
        cr.JNEL(LOOP_START),

        cr.HALT()
    );

    for (auto _ : state) {
        auto vm = interpreter(arch);
        vm.run(code);
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R1));
    }
}

BENCHMARK_F(VirtualMachineBenchmark, Rc4KeystreamXor_Native)(benchmark::State& state) {
    static constexpr std::uint8_t KEY[] = {'K', 'e', 'y'};
    static constexpr std::size_t KETLEN = sizeof KEY;
    static constexpr std::uint8_t PLAINTEXT_BYTE = 'P';

    for (auto _ : state) {
        constexpr auto keystream = crypto::rc4_keystream_byte(KEY, KETLEN);
        std::uint8_t cipher = PLAINTEXT_BYTE ^ keystream;
        std::uint8_t decrypted = cipher ^ keystream;
        benchmark::DoNotOptimize(cipher);
        benchmark::DoNotOptimize(decrypted);
        benchmark::ClobberMemory();
    }
}

BENCHMARK_F(VirtualMachineBenchmark, Rc4KeystreamXor_VM)(benchmark::State& state) {
    static constexpr std::uint8_t KEY[] = {'K', 'e', 'y'};
    static constexpr std::size_t KETLEN = sizeof KEY;
    static constexpr std::uint8_t PLAINTEXT_BYTE = 'P';
    static constexpr std::uint8_t EXPECTED_KEYSTREAM = crypto::rc4_keystream_byte(KEY, KETLEN);

    static constexpr auto code = PVM_ASSEMBLE(
        arch,
        cr.MOV(arch::reg::REG_R1, operand(static_cast<std::uint64_t>(PLAINTEXT_BYTE))),
        cr.MOV(arch::reg::REG_R2, operand(static_cast<std::uint64_t>(EXPECTED_KEYSTREAM))),

        // R3 = encrypt(plaintext) = plaintext ^ keystream
        cr.MOV(arch::reg::REG_R3, operand(arch::reg::REG_R1)),
        cr.XOR(arch::reg::REG_R3, operand(arch::reg::REG_R2)),

        // R4 = decrypt(ciphertext) = ciphertext ^ keystream
        cr.MOV(arch::reg::REG_R4, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R4, operand(arch::reg::REG_R2)),

        // R5 = round-trip: encrypt then decrypt
        cr.MOV(arch::reg::REG_R5, operand(arch::reg::REG_R1)),
        cr.XOR(arch::reg::REG_R5, operand(arch::reg::REG_R2)),
        cr.XOR(arch::reg::REG_R5, operand(arch::reg::REG_R2)),

        cr.HALT()
    );

    for (auto _ : state) {
        auto vm = interpreter(arch);
        vm.run(code);
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R3));
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R4));
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R5));
    }
}

BENCHMARK_F(VirtualMachineBenchmark, ChaCha20QuarterRound_Native)(benchmark::State& state) {
    static constexpr std::uint32_t INPUT_A = 0x11111111u;
    static constexpr std::uint32_t INPUT_B = 0x01020304u;
    static constexpr std::uint32_t INPUT_C = 0x9b8d6f43u;
    static constexpr std::uint32_t INPUT_D = 0x01234567u;

    std::uint32_t a = INPUT_A, b = INPUT_B, c = INPUT_C, d = INPUT_D;

    for (auto _ : state) {
        crypto::chacha20_quarter_round(a, b, c, d);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(c);
        benchmark::DoNotOptimize(d);
        a = INPUT_A;
        b = INPUT_B;
        c = INPUT_C;
        d = INPUT_D;
    }
}

BENCHMARK_F(VirtualMachineBenchmark, ChaCha20QuarterRound_VM)(benchmark::State& state) {
    static constexpr std::uint32_t INPUT_A = 0x11111111u;
    static constexpr std::uint32_t INPUT_B = 0x01020304u;
    static constexpr std::uint32_t INPUT_C = 0x9b8d6f43u;
    static constexpr std::uint32_t INPUT_D = 0x01234567u;

    // R1=a, R2=b, R3=c, R4=d, R5/R6=scratch for rotl32
    static constexpr auto code = PVM_ASSEMBLE(
        arch,
        cr.MOV(arch::reg::REG_R1, operand(INPUT_A)),
        cr.MOV(arch::reg::REG_R2, operand(INPUT_B)),
        cr.MOV(arch::reg::REG_R3, operand(INPUT_C)),
        cr.MOV(arch::reg::REG_R4, operand(INPUT_D)),

        // a += b; d ^= a; d = rotl32(d, 16)
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.AND(arch::reg::REG_R1, operand(0xFFFFFFFFu)),
        cr.XOR(arch::reg::REG_R4, operand(arch::reg::REG_R1)),
        cr.AND(arch::reg::REG_R4, operand(0xFFFFFFFFu)),
        cr.MOV(arch::reg::REG_R5, operand(arch::reg::REG_R4)),
        cr.SHL(arch::reg::REG_R5, operand(16u)),
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R4)),
        cr.SHR(arch::reg::REG_R6, operand(16u)),
        cr.OR(arch::reg::REG_R5, operand(arch::reg::REG_R6)),
        cr.MOV(arch::reg::REG_R4, operand(arch::reg::REG_R5)),
        cr.AND(arch::reg::REG_R4, operand(0xFFFFFFFFu)),

        // c += d; b ^= c; b = rotl32(b, 12)
        cr.ADD(arch::reg::REG_R3, operand(arch::reg::REG_R4)),
        cr.AND(arch::reg::REG_R3, operand(0xFFFFFFFFu)),
        cr.XOR(arch::reg::REG_R2, operand(arch::reg::REG_R3)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),
        cr.MOV(arch::reg::REG_R5, operand(arch::reg::REG_R2)),
        cr.SHL(arch::reg::REG_R5, operand(12u)),
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R2)),
        cr.SHR(arch::reg::REG_R6, operand(20u)),
        cr.OR(arch::reg::REG_R5, operand(arch::reg::REG_R6)),
        cr.MOV(arch::reg::REG_R2, operand(arch::reg::REG_R5)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),

        // a += b; d ^= a; d = rotl32(d, 8)
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.AND(arch::reg::REG_R1, operand(0xFFFFFFFFu)),
        cr.XOR(arch::reg::REG_R4, operand(arch::reg::REG_R1)),
        cr.AND(arch::reg::REG_R4, operand(0xFFFFFFFFu)),
        cr.MOV(arch::reg::REG_R5, operand(arch::reg::REG_R4)),
        cr.SHL(arch::reg::REG_R5, operand(8u)),
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R4)),
        cr.SHR(arch::reg::REG_R6, operand(24u)),
        cr.OR(arch::reg::REG_R5, operand(arch::reg::REG_R6)),
        cr.MOV(arch::reg::REG_R4, operand(arch::reg::REG_R5)),
        cr.AND(arch::reg::REG_R4, operand(0xFFFFFFFFu)),

        // c += d; b ^= c; b = rotl32(b, 7)
        cr.ADD(arch::reg::REG_R3, operand(arch::reg::REG_R4)),
        cr.AND(arch::reg::REG_R3, operand(0xFFFFFFFFu)),
        cr.XOR(arch::reg::REG_R2, operand(arch::reg::REG_R3)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),
        cr.MOV(arch::reg::REG_R5, operand(arch::reg::REG_R2)),
        cr.SHL(arch::reg::REG_R5, operand(7u)),
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R2)),
        cr.SHR(arch::reg::REG_R6, operand(25u)),
        cr.OR(arch::reg::REG_R5, operand(arch::reg::REG_R6)),
        cr.MOV(arch::reg::REG_R2, operand(arch::reg::REG_R5)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),

        cr.HALT()
    );

    for (auto _ : state) {
        auto vm = interpreter(arch);
        vm.run(code);
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R1));
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R2));
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R3));
        benchmark::DoNotOptimize(vm.get_ctx()->get_reg(arch::reg::REG_R4));
    }
}

#endif // NGU_PVM_TESTS_BENCHMARK_H
