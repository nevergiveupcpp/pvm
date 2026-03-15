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

#ifndef NGU_PVM_TESTS_UNITTEST_MACRO_H
#define NGU_PVM_TESTS_UNITTEST_MACRO_H

#include "unittest_fixture.h"

TEST_F(VirtualMachineTest, MacroZero) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xDEADBEEFu)),
        cr.ZERO(arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0u);
}

TEST_F(VirtualMachineTest, MacroNeg) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.NEG(arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), static_cast<std::uint64_t>(-5));
}

TEST_F(VirtualMachineTest, MacroNegZero) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0u)),
        cr.NEG(arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0u);
}

TEST_F(VirtualMachineTest, MacroMovc) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xABCDu)),
        cr.MOV(arch::reg::REG_R2, operand(0xFFFFu)),
        cr.MOVC(arch::reg::REG_R2, arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0xABCDu);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0xABCDu);
}

TEST_F(VirtualMachineTest, MacroMov64) {
    constexpr std::uint32_t HI = 0xDEADBEEFu;
    constexpr std::uint32_t LO = 0xCAFEBABEu;
    constexpr std::uint64_t EXPECTED = (static_cast<std::uint64_t>(HI) << 32) | LO;

    // MOV64(dst, hi, lo) expands to: MOV(dst, hi); SHL(dst, 32); OR(dst, lo)
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(HI)),
        cr.SHL(arch::reg::REG_R1, operand(32u)),
        cr.OR(arch::reg::REG_R1, operand(LO)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), EXPECTED);
}

TEST_F(VirtualMachineTest, MacroSwap) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xAAAAu)),
        cr.MOV(arch::reg::REG_R2, operand(0x5555u)),
        cr.SWAP(arch::reg::REG_R1, arch::reg::REG_R2),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0x5555u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0xAAAAu);
}

TEST_F(VirtualMachineTest, MacroLoop) {
    // Counts from 5 down to 0, adding 1 to an accumulator each iteration.
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0u)),   // accumulator
        cr.MOV(arch::reg::REG_R2, operand(5u)),   // loop counter
        cr.LABEL(1),
        cr.ADD(arch::reg::REG_R1, operand(1u)),
        cr.LOOP(arch::reg::REG_R2, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 5u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0u);
}

TEST_F(VirtualMachineTest, MacroMulPow2) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(7u)),
        cr.MUL_POW2(arch::reg::REG_R1, 3),  // 7 * 2^3 = 56
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 56u);
}

TEST_F(VirtualMachineTest, MacroDivPow2) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(64u)),
        cr.DIV_POW2(arch::reg::REG_R1, 3),  // 64 / 2^3 = 8
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 8u);
}

TEST_F(VirtualMachineTest, MacroMaskLo) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xABCDEF01u)),
        cr.MASK_LO(arch::reg::REG_R1, 8),   // keep low 8 bits → 0x01
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0x01u);
}

TEST_F(VirtualMachineTest, MacroAbsPositive) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(42u)),
        cr.ABS(arch::reg::REG_R1, arch::reg::REG_R2, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 42u);
}

TEST_F(VirtualMachineTest, MacroAbsNegative) {
    // Pass as uint64 to force DWORD_Q encoding: immediate is zero-extended on decode,
    // so signed-compressed encoding (-7 → 0xF9) would read back as 249, not -7.
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(static_cast<std::uint64_t>(-7LL))),
        cr.ABS(arch::reg::REG_R1, arch::reg::REG_R2, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 7u);
}

TEST_F(VirtualMachineTest, MacroAbsZero) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0u)),
        cr.ABS(arch::reg::REG_R1, arch::reg::REG_R2, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0u);
}

TEST_F(VirtualMachineTest, MacroMinALessB) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(3u)),
        cr.MOV(arch::reg::REG_R2, operand(7u)),
        cr.MIN(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 3u);
}

TEST_F(VirtualMachineTest, MacroMinAGreaterB) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(9u)),
        cr.MOV(arch::reg::REG_R2, operand(4u)),
        cr.MIN(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 4u);
}

TEST_F(VirtualMachineTest, MacroMinEqual) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.MOV(arch::reg::REG_R2, operand(5u)),
        cr.MIN(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 5u);
}

TEST_F(VirtualMachineTest, MacroMaxAGreaterB) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(9u)),
        cr.MOV(arch::reg::REG_R2, operand(4u)),
        cr.MAX(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 9u);
}

TEST_F(VirtualMachineTest, MacroMaxALessB) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(3u)),
        cr.MOV(arch::reg::REG_R2, operand(7u)),
        cr.MAX(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 7u);
}

TEST_F(VirtualMachineTest, MacroMaxEqual) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.MOV(arch::reg::REG_R2, operand(5u)),
        cr.MAX(arch::reg::REG_R1, arch::reg::REG_R2, arch::reg::REG_R3, 1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 5u);
}

#endif //NGU_PVM_TESTS_UNITTEST_MACRO_H
