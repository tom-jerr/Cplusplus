  .section .text.entry
  .globl _start
_start:
  # 建立栈空间
  la sp, boot_stack_top
  call main
  # bss 段清零
  .section .bss.stack
  .globl boot_stack
boot_stack:
  .space 4096 * 16
  .globl boot_stack_top
boot_stack_top: