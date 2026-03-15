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

#ifndef NGU_PVM_TESTS_UNITTEST_FIXTURE_H
#define NGU_PVM_TESTS_UNITTEST_FIXTURE_H

#include <gtest/gtest.h>

#include "pvm/codegen/macro_assembler.h"
#include "pvm/codegen/codegen.h"
#include "pvm/engine/interpreter.h"

using namespace ngu::pvm;

class VirtualMachineTest : public ::testing::Test {
protected:
    static constexpr auto arch = architecture::make(1);
    static constexpr auto cr   = macro_assembler(arch);
};

#endif //NGU_PVM_TESTS_UNITTEST_FIXTURE_H
