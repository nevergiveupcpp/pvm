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

#ifndef NGU_PVM_TESTS_UNITTEST_CRYPTO_H
#define NGU_PVM_TESTS_UNITTEST_CRYPTO_H

#include "unittest_fixture.h"

#include "../assets/crypto.h"

// Encrypts a known plaintext with the C++ reference, then runs the XTEA
// decryption algorithm in VM bytecode and verifies the registers hold the
// original plaintext values.
TEST_F(VirtualMachineTest, XteaDecryptRoundTrip) {
    constexpr std::uint32_t PLAIN_V0 = 0x01234567u;
    constexpr std::uint32_t PLAIN_V1 = 0x89ABCDEFu;
    static constexpr std::uint32_t KEY[4] = {0x00112233u, 0x44556677u, 0x8899AABBu, 0xCCDDEEFFu};

    static constexpr auto cipher = crypto::xtea_encrypt(PLAIN_V0, PLAIN_V1, KEY);

    constexpr std::uint64_t LOOP_START   = 1;
    constexpr std::uint64_t SWITCH1_KEY1 = 2;
    constexpr std::uint64_t SWITCH1_KEY2 = 3;
    constexpr std::uint64_t SWITCH1_KEY3 = 4;
    constexpr std::uint64_t SWITCH1_END  = 5;
    constexpr std::uint64_t SWITCH2_KEY1 = 6;
    constexpr std::uint64_t SWITCH2_KEY2 = 7;
    constexpr std::uint64_t SWITCH2_KEY3 = 8;
    constexpr std::uint64_t SWITCH2_END  = 9;

    // R1=v0(cipher), R2=v1(cipher), R3=sum, R4=delta, R5=rounds_counter
    // R6/R7/R8 = scratch
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(cipher.first)),
        cr.MOV(arch::reg::REG_R2, operand(cipher.second)),
        cr.MOV(arch::reg::REG_R3, operand(crypto::SUM_INIT)),
        cr.MOV(arch::reg::REG_R4, operand(crypto::XTEA_DELTA)),
        cr.MOV(arch::reg::REG_R5, operand(crypto::XTEA_ROUNDS)),

        cr.LABEL(LOOP_START),

        // --- update v1 ---
        // R6 = (v0 << 4) ^ (v0 >> 5) ^ v0
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R1)),
        cr.SHL(arch::reg::REG_R6, operand(4u)),
        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R1)),
        cr.SHR(arch::reg::REG_R7, operand(5u)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R7)),
        cr.ADD(arch::reg::REG_R6, operand(arch::reg::REG_R1)),

        // R7 = (sum >> 11) & 3  → key index for v1 update
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

        // v1 -= ((v0<<4 ^ v0>>5) + v0) ^ (sum + key[(sum>>11)&3])
        cr.ADD(arch::reg::REG_R8, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R8)),
        cr.SUB(arch::reg::REG_R2, operand(arch::reg::REG_R6)),
        cr.AND(arch::reg::REG_R2, operand(0xFFFFFFFFu)),

        cr.SUB(arch::reg::REG_R3, operand(arch::reg::REG_R4)),
        cr.AND(arch::reg::REG_R3, operand(0xFFFFFFFFu)),

        // --- update v0 ---
        // R6 = (v1 << 4) ^ (v1 >> 5) ^ v1
        cr.MOV(arch::reg::REG_R6, operand(arch::reg::REG_R2)),
        cr.SHL(arch::reg::REG_R6, operand(4u)),
        cr.MOV(arch::reg::REG_R7, operand(arch::reg::REG_R2)),
        cr.SHR(arch::reg::REG_R7, operand(5u)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R7)),
        cr.ADD(arch::reg::REG_R6, operand(arch::reg::REG_R2)),

        // R7 = sum & 3  → key index for v0 update
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

        // v0 -= ((v1<<4 ^ v1>>5) + v1) ^ (sum + key[sum&3])
        cr.ADD(arch::reg::REG_R8, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R6, operand(arch::reg::REG_R8)),
        cr.SUB(arch::reg::REG_R1, operand(arch::reg::REG_R6)),
        cr.AND(arch::reg::REG_R1, operand(0xFFFFFFFFu)),

        cr.SUB(arch::reg::REG_R5, operand(1u)),
        cr.CMP(arch::reg::REG_R5, operand(0u)),
        cr.JNEL(LOOP_START),

        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    auto status = test_vm.run(code);

    ASSERT_EQ(status, interpreter::status::VM_SUCCESS);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R1)), PLAIN_V0);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R2)), PLAIN_V1);
}

// Verifies that the VM can apply the RC4 keystream XOR idiom:
//   encrypt:  ciphertext  = plaintext  ^ keystream
//   decrypt:  plaintext   = ciphertext ^ keystream
// The keystream byte is pre-computed at compile time by the reference
// implementation so the test stays self-contained and easy to read.
TEST_F(VirtualMachineTest, Rc4KeystreamXor) {
    constexpr std::uint8_t KEY[]    = {'K', 'e', 'y'};
    constexpr std::size_t  KETLEN   = sizeof KEY;
    constexpr std::uint8_t PLAIN    = 'P';
    constexpr std::uint8_t KEYSTREAM  = crypto::rc4_keystream_byte(KEY, KETLEN);
    constexpr std::uint8_t EXPECTED_CIPHER = crypto::rc4_process(KEY, KETLEN, PLAIN);

    static_assert(EXPECTED_CIPHER == (PLAIN ^ KEYSTREAM));

    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(static_cast<std::uint64_t>(PLAIN))),
        cr.MOV(arch::reg::REG_R2, operand(static_cast<std::uint64_t>(KEYSTREAM))),

        // R3 = encrypt(plaintext) = plaintext ^ keystream
        cr.MOV(arch::reg::REG_R3, operand(arch::reg::REG_R1)),
        cr.XOR(arch::reg::REG_R3, operand(arch::reg::REG_R2)),

        // R4 = decrypt(ciphertext) = ciphertext ^ keystream
        cr.MOV(arch::reg::REG_R4, operand(arch::reg::REG_R3)),
        cr.XOR(arch::reg::REG_R4, operand(arch::reg::REG_R2)),

        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    auto status = test_vm.run(code);

    ASSERT_EQ(status, interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R3), EXPECTED_CIPHER);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R4), PLAIN);
}

// Runs the ChaCha20 quarter-round in VM bytecode and compares against the
// compile-time reference.  Rotations are implemented as (SHL | SHR) because
// the VM's ROL/ROR operate on 64-bit registers, while the ChaCha20 spec
// requires 32-bit rotations.
TEST_F(VirtualMachineTest, ChaCha20QuarterRound) {
    constexpr std::uint32_t INPUT_A = 0x11111111u;
    constexpr std::uint32_t INPUT_B = 0x01020304u;
    constexpr std::uint32_t INPUT_C = 0x9b8d6f43u;
    constexpr std::uint32_t INPUT_D = 0x01234567u;

    constexpr auto expected = crypto::chacha20_qr(INPUT_A, INPUT_B, INPUT_C, INPUT_D);
    constexpr std::uint32_t EXPECTED_A = std::get<0>(expected);
    constexpr std::uint32_t EXPECTED_B = std::get<1>(expected);
    constexpr std::uint32_t EXPECTED_C = std::get<2>(expected);
    constexpr std::uint32_t EXPECTED_D = std::get<3>(expected);

    // R1=a, R2=b, R3=c, R4=d, R5/R6=scratch for rotl32
    constexpr auto code = PVM_ASSEMBLE(arch,
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

    auto test_vm = interpreter(arch);
    auto status = test_vm.run(code);

    ASSERT_EQ(status, interpreter::status::VM_SUCCESS);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R1)), EXPECTED_A);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R2)), EXPECTED_B);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R3)), EXPECTED_C);
    EXPECT_EQ(static_cast<std::uint32_t>(test_vm.get_ctx()->get_reg(arch::reg::REG_R4)), EXPECTED_D);
}

#endif //NGU_PVM_TESTS_UNITTEST_CRYPTO_H
