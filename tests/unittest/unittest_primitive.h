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

#ifndef NGU_PVM_TESTS_UNITTEST_PRIMITIVE_H
#define NGU_PVM_TESTS_UNITTEST_PRIMITIVE_H

#include "unittest_fixture.h"

TEST_F(VirtualMachineTest, Initialization) {
    constexpr auto test_vm = interpreter(arch);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R3), 0);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R4), 0);

    EXPECT_EQ(test_vm.get_ctx()->get_flags(), 0);
    EXPECT_EQ(test_vm.get_ctx()->get_pc(), 0);
}

TEST_F(VirtualMachineTest, MovImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(42u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 42u);
}

TEST_F(VirtualMachineTest, MovRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xBEEFu)),
        cr.MOV(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0xBEEFu);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0xBEEFu);
}

TEST_F(VirtualMachineTest, BoundaryValues) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0u)),
        cr.MOV(arch::reg::REG_R2, operand(UINT64_MAX)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), UINT64_MAX);
}

TEST_F(VirtualMachineTest, AllRegistersIndependent) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(1u)),
        cr.MOV(arch::reg::REG_R2, operand(2u)),
        cr.MOV(arch::reg::REG_R3, operand(3u)),
        cr.MOV(arch::reg::REG_R4, operand(4u)),
        cr.MOV(arch::reg::REG_R5, operand(5u)),
        cr.MOV(arch::reg::REG_R6, operand(6u)),
        cr.MOV(arch::reg::REG_R7, operand(7u)),
        cr.MOV(arch::reg::REG_R8, operand(8u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 1u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 2u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R3), 3u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R4), 4u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R5), 5u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R6), 6u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R7), 7u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R8), 8u);
}

TEST_F(VirtualMachineTest, AddImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.ADD(arch::reg::REG_R1, operand(5u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 15u);
}

TEST_F(VirtualMachineTest, AddRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.MOV(arch::reg::REG_R2, operand(5u)),
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 15u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 5u);
}

TEST_F(VirtualMachineTest, AddOverflow) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(UINT64_MAX - 5)),
        cr.ADD(arch::reg::REG_R1, operand(10u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 4u);
}

TEST_F(VirtualMachineTest, SubImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(20u)),
        cr.SUB(arch::reg::REG_R1, operand(7u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 13u);
}

TEST_F(VirtualMachineTest, SubRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(20u)),
        cr.MOV(arch::reg::REG_R2, operand(7u)),
        cr.SUB(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 13u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 7u);
}

TEST_F(VirtualMachineTest, SubUnderflow) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.SUB(arch::reg::REG_R1, operand(10u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), UINT64_MAX - 4);
}

TEST_F(VirtualMachineTest, AndImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11110000u)),
        cr.AND(arch::reg::REG_R1, operand(0b10101010u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b10100000u);
}

TEST_F(VirtualMachineTest, AndRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xFFu)),
        cr.MOV(arch::reg::REG_R2, operand(0x0Fu)),
        cr.AND(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0x0Fu);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0x0Fu);
}

TEST_F(VirtualMachineTest, OrImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11110000u)),
        cr.OR(arch::reg::REG_R1, operand(0b10101010u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b11111010u);
}

TEST_F(VirtualMachineTest, OrRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11110000u)),
        cr.MOV(arch::reg::REG_R2, operand(0b00001111u)),
        cr.OR(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b11111111u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0b00001111u);
}

TEST_F(VirtualMachineTest, XorImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11110000u)),
        cr.XOR(arch::reg::REG_R1, operand(0b10101010u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b01011010u);
}

TEST_F(VirtualMachineTest, XorRegister) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11001100u)),
        cr.MOV(arch::reg::REG_R2, operand(0b10101010u)),
        cr.XOR(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b01100110u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0b10101010u);
}

TEST_F(VirtualMachineTest, XorSelf) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xFFu)),
        cr.XOR(arch::reg::REG_R1, operand(arch::reg::REG_R1)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0u);
}

TEST_F(VirtualMachineTest, NotImmediate) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b10101010u)),
        cr.NOT(arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), ~static_cast<std::uint64_t>(0b10101010u));
}

TEST_F(VirtualMachineTest, NotZero) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0u)),
        cr.NOT(arch::reg::REG_R1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), UINT64_MAX);
}

TEST_F(VirtualMachineTest, ShiftLeft) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.SHL(arch::reg::REG_R1, operand(2u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 20u);
}

TEST_F(VirtualMachineTest, ShiftLeftOverflow) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(1u)),
        cr.SHL(arch::reg::REG_R1, operand(63u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0x8000000000000000ULL);
}

TEST_F(VirtualMachineTest, ShiftRight) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(20u)),
        cr.SHR(arch::reg::REG_R1, operand(2u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 5u);
}

TEST_F(VirtualMachineTest, ShiftRightBoundary) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0x8000000000000000ULL)),
        cr.SHR(arch::reg::REG_R1, operand(63u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 1u);
}

TEST_F(VirtualMachineTest, RotateLeft) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0x8000000000000001ULL)),
        cr.ROL(arch::reg::REG_R1, operand(1u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0x3u);
}

TEST_F(VirtualMachineTest, RotateRight) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0x8000000000000001ULL)),
        cr.ROR(arch::reg::REG_R1, operand(1u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0xC000000000000000ULL);
}

TEST_F(VirtualMachineTest, CompareEqual) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(42u)),
        cr.CMP(arch::reg::REG_R1, operand(42u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    auto flags = test_vm.get_ctx()->get_flags();
    EXPECT_NE(flags & 0x1, 0u);
    EXPECT_EQ(flags & 0x2, 0u);
}

TEST_F(VirtualMachineTest, CompareNotEqual) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(42u)),
        cr.CMP(arch::reg::REG_R1, operand(24u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_flags() & 0x1, 0u);
}

TEST_F(VirtualMachineTest, CompareLess) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.CMP(arch::reg::REG_R1, operand(20u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    auto flags = test_vm.get_ctx()->get_flags();
    EXPECT_NE(flags & 0x2, 0u);
    EXPECT_EQ(flags & 0x1, 0u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 10u);
}

TEST_F(VirtualMachineTest, CompareGreater) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(20u)),
        cr.CMP(arch::reg::REG_R1, operand(10u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    auto flags = test_vm.get_ctx()->get_flags();
    EXPECT_EQ(flags & 0x1, 0u);
    EXPECT_EQ(flags & 0x2, 0u);
}

TEST_F(VirtualMachineTest, JumpUnconditional) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.JMPL(1),
        cr.MOV(arch::reg::REG_R2, operand(99u)),
        cr.LABEL(1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 10u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0u);
}

TEST_F(VirtualMachineTest, JumpIndexedUnconditional) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.MOV(arch::reg::REG_R2, operand(10u)),
        cr.JMPI(operand(18u)),
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R2, operand(arch::reg::REG_R1)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 10u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 10u);
}

TEST_F(VirtualMachineTest, JumpLabel) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R2, operand(100u)),
        cr.JMPL(1),
        cr.MOV(arch::reg::REG_R2, operand(101u)),
        cr.LABEL(1),
        cr.MOV(arch::reg::REG_R3, operand(102u)),
        cr.JMPL(2),
        cr.ADD(arch::reg::REG_R3, operand(200u)),
        cr.XOR(arch::reg::REG_R3, operand(5u)),
        cr.XOR(arch::reg::REG_R3, operand(arch::reg::REG_R2)),
        cr.LABEL(2),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 100u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R3), 102u);
}

TEST_F(VirtualMachineTest, JumpIfEqualTaken) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.CMP(arch::reg::REG_R1, operand(5u)),
        cr.JEL(1),
        cr.MOV(arch::reg::REG_R2, operand(99u)),
        cr.LABEL(1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0u);
}

TEST_F(VirtualMachineTest, JumpIfEqualNotTaken) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.CMP(arch::reg::REG_R1, operand(10u)),
        cr.JEL(1),
        cr.MOV(arch::reg::REG_R2, operand(99u)),
        cr.LABEL(1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 99u);
}

TEST_F(VirtualMachineTest, JumpIfNotEqualTaken) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.CMP(arch::reg::REG_R1, operand(10u)),
        cr.JNEL(1),
        cr.MOV(arch::reg::REG_R2, operand(99u)),
        cr.LABEL(1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 0u);
}

TEST_F(VirtualMachineTest, JumpIfNotEqualNotTaken) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(5u)),
        cr.CMP(arch::reg::REG_R1, operand(5u)),
        cr.JNEL(1),
        cr.MOV(arch::reg::REG_R2, operand(99u)),
        cr.LABEL(1),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 99u);
}

TEST_F(VirtualMachineTest, NopInstruction) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(42u)),
        cr.NOP(),
        cr.NOP(),
        cr.NOP(),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 42u);
}

TEST_F(VirtualMachineTest, HaltStatus) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(1u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    EXPECT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);
}

TEST_F(VirtualMachineTest, ArithmeticChain) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(100u)),
        cr.ADD(arch::reg::REG_R1, operand(50u)),
        cr.SUB(arch::reg::REG_R1, operand(25u)),
        cr.SHL(arch::reg::REG_R1, operand(1u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 250u);
}

TEST_F(VirtualMachineTest, LogicalChain) {
    // 0xFF & 0xF0 = 0xF0, | 0x0A = 0xFA, ^ 0x55 = 0xAF
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0xFFu)),
        cr.AND(arch::reg::REG_R1, operand(0xF0u)),
        cr.OR(arch::reg::REG_R1, operand(0x0Au)),
        cr.XOR(arch::reg::REG_R1, operand(0x55u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0xAFu);
}

TEST_F(VirtualMachineTest, RegisterToRegisterOperations) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(10u)),
        cr.MOV(arch::reg::REG_R2, operand(20u)),
        cr.MOV(arch::reg::REG_R3, operand(arch::reg::REG_R1)),
        cr.ADD(arch::reg::REG_R3, operand(arch::reg::REG_R2)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 10u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R2), 20u);
    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R3), 30u);
}

TEST_F(VirtualMachineTest, MultiRegisterOperations) {
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(1u)),
        cr.MOV(arch::reg::REG_R2, operand(2u)),
        cr.MOV(arch::reg::REG_R3, operand(3u)),
        cr.MOV(arch::reg::REG_R4, operand(4u)),
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.ADD(arch::reg::REG_R3, operand(arch::reg::REG_R4)),
        cr.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R3)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 10u);
}

TEST_F(VirtualMachineTest, BitwiseCombo) {
    // (0b11001100 & 0b10101010) = 0b10001000; | 0b01010101 = 0b11011101
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(0b11001100u)),
        cr.MOV(arch::reg::REG_R2, operand(0b10101010u)),
        cr.AND(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        cr.MOV(arch::reg::REG_R3, operand(0b01010101u)),
        cr.OR(arch::reg::REG_R1, operand(arch::reg::REG_R3)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 0b11011101u);
}

TEST_F(VirtualMachineTest, ShiftChain) {
    // 1 << 4 = 16, << 2 = 64, >> 3 = 8
    constexpr auto code = PVM_ASSEMBLE(arch,
        cr.MOV(arch::reg::REG_R1, operand(1u)),
        cr.SHL(arch::reg::REG_R1, operand(4u)),
        cr.SHL(arch::reg::REG_R1, operand(2u)),
        cr.SHR(arch::reg::REG_R1, operand(3u)),
        cr.HALT()
    );

    auto test_vm = interpreter(arch);
    ASSERT_EQ(test_vm.run(code), interpreter::status::VM_SUCCESS);

    EXPECT_EQ(test_vm.get_ctx()->get_reg(arch::reg::REG_R1), 8u);
}

#endif //NGU_PVM_TESTS_UNITTEST_PRIMITIVE_H
