# Bare-metal UBSAN implementation

*This is not an officially supported Google product.*

This project demonstrates how to enable Undefined Behaviour Sanitizer (KUBSan) for
bare-metal code running on (currently) RISC-V-64 architectures. It implements a
set of UBSan test cases that catch various classes of undefined behaviour bugs
at runtime.

The implementation of KASan run-time routines in this project is inspired by
the corresponding implementation of UBSan in Linux Kernel, and functions similarly to the Bare-metal KASAn implementation (https://github.com/androidoffsec/baremetal_kasan) as this project is based on a fork of the repo.

Please be aware that this implementation could be futher improved, either by developing more handlers to the various UB cases or by developing a version of the project with both KASan and KUBSan implemented.

## Prerequisites

To build and run the program you would need to use LLVM toolchain (`clang` and
`ld.lld`) for cross complitation and QEMU system emulator for supported
architectures.

For example, here are the necessary Debian package names needed to build and
run the project:

```
sudo apt-get install build-essential gcc-multilib llvm clang lld \
                     qemu-system-arm qemu-system-misc qemu-system-x86
```

## Project layout

The project constists of the following components, please note that most of these files are imported from the KASAn bare metal implementation.:

* `kubsan_test.c` -- main test driver which runs (in this case, UBsan) test cases
* `sanitized_lib.c` -- this module implements the test cases and is built with
                    the UBSan instrumentation
* `kubsan.c` -- implementation of runtime routines needed for KUBsan sanitizer
* `heap.c` -- simple implementation of heap management routines for testing
              UBSan
* `third_party/printf.c` -- a compact implementation of `printf` function
* `rt_utils.c` -- run-time utility functions
* `start_arch.S` -- architecture-specific low-level entry point in assembly
                    for the bare-metal program
* `kasan_test.ld` -- linker script for the program
* `Makefile` -- in addition to instructions on how to build and run the project
              this file contains definitions of some important parameters,
              such as KASan shadow memory address, DRAM start address and KASan
              configuration options. Most of these parameters are not needed in the UBSan implementation.
* `Makefile.arch` -- Makefile fragments whith architecture-specific parameters
                     for building and running the project in the emulator


## Running

To build and execute the test suite run `ARCH=target_arch make clean run`
where `target_arch` is one of the supported architectures: `arm`, `aarch64`,
`riscv32`, `riscv64` and `x86`. If the target architecture isn't specified
(i.e. `make clean run`) then `arm` is assumed as default option.

As an example, running `make clean run` should build the bare-metal program
and execute it in QEMU ARM system emulator with the following expected output:

```
qemu-system-arm -M virt-8.2 -cpu cortex-a7 -m 256M -nographic -kernel kasan_test
Starting bare-metal KASan test driver.

KUBSan test: integer overflow in add operation
Overflowing integer variable with INT_MAX value
================================================================================
UBSAN: signed-integer-overflow in ./sanitized_lib.c:41:6
+ operation cannot be represented in type 'int' with those operands
----------------------

KUBSan test: integer underflow in sub operation
Underflowing integer variable with INT_MIN value
================================================================================
UBSAN: signed-integer-overflow in ./sanitized_lib.c:52:6
- operation cannot be represented in type 'int' with those operands
----------------------

KUBSan test: integer overflow in mul operation
Overflowing integer variable with INT_MAX value
================================================================================
UBSAN: signed-integer-overflow in ./sanitized_lib.c:64:6
* operation cannot be represented in type 'int' with those operands
----------------------

KUBSan test: integer negate overflow
Overflowing by negating integer variable with INT_MIN value
================================================================================
UBSAN: negation-overflow in ./sanitized_lib.c:74:8
negation of this value cannot be represented in type 'int'
----------------------

KUBSan test: integer division by zero
Dividing by zero variable with 16 value
================================================================================
UBSAN: division-overflow in ./sanitized_lib.c:85:6
division by zero
----------------------