OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(reset_handler)
SECTIONS
{
  . = 0x80000000;
  .start : { *(.reset) }
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .init_array : {
    __global_ctors_start = .;
    *(.init_array*)
    __global_ctors_end = .;
 }
  .data : { *(.data) }
  .bss : { *(.bss COMMON) }
  . = ALIGN(0x1000);
  __heap_start = .;
  . = . + 0x10000;
  __heap_end = .;
  . = . + 0x1000;
  __stack_top = .;
}
