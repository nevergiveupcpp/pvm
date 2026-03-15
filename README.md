# PVM (Polymorphic Virtual Machine)

<p align="center">
  <img src="images/banner.png">
</p>

## Description
**The project was created to support virtualization in the “obfuscxx” project and potentially for broader use in the future.**

PVM provides an embeddable, compile-time polymorphic register-based virtual machine that enables virtualization of cryptographic primitives through manual construction of assembly listings.

In the context of this project, by polymorphism we mean the virtual machine’s ability to alter its instruction set architecture (ISA) based on a provided seed. In other words, each VM instance has a unique set of opcode and register indices.

PVM offers a wide range of classic instructions and also includes a Macro Assembler implementation that greatly simplifies code writing. All bytecode is assembled at compile time and can be placed either on the stack or as a global variable. At runtime, only the bytecode dispatching routines remain.

Currently, some ISA and ABI details have been completely removed from the project. For example, the current implementation does not include CALL or RET instructions, and there is no stack or context preservation at all. This deliberate simplification serves a single purpose - integrating virtualization into obfuscxx, which is the original reason this project was created.

## Installation
Copy the [include](include) directory into your project and include the master header.

```c++
#include <pvm/pvm.h>
```

Alternatively, you can include individual subsystems directly.

```c++
#include <pvm/engine/interpreter.h>
#include <pvm/codegen/macro_assembler.h>
```

## Examples
```c++
    using namespace ngu::pvm;

    static constexpr auto arch = architecture::make(12345);
    static constexpr auto masm = macro_assembler(arch);

    constexpr auto vm = interpreter(arch);

    constexpr auto code = PVM_ASSEMBLE(arch,
        masm.MOV(arch::reg::REG_R1, operand(40)),
        masm.MOV(arch::reg::REG_R2, operand(2)),
        masm.ADD(arch::reg::REG_R1, operand(arch::reg::REG_R2)),
        masm.HALT()
    );

    auto const status = vm.run(code);

    if (status == interpreter::status::VM_SUCCESS) {
        auto const r1 = vm.get_ctx()->get_reg(arch::reg::REG_R1);
        std::cout << r1 << std::endl;
    }
```
> To see more advanced implementation examples, check out the [tests](tests) directory.

## Building tests and benchmarks
1. Install `vcpkg` and set `VCPKG_ROOT` environment variable
2. Fetch baseline: `cd $VCPKG_ROOT && git fetch origin 34823ada10080ddca99b60e85f80f55e18a44eea`
3. Configure: `cmake --preset <compiler>` (MSVC/Clang/GCC)
4. Build: `cmake --build --preset <compiler>` (--config Release/Debug)

## Requirements
- C++20 or later
- CMake 3.15+ (for building tests)
- vcpkg (for dependencies)

## Platform Support

### Compilers
- MSVC (+ WDM)
- GCC
- Clang

### Architectures
- x86
- x86-64

### Operating Systems
- Windows
- Linux

## License
**pvm** is distributed under the [Apache License 2.0](LICENSE).
