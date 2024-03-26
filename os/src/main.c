#include "../include/printf.h"

extern char stext, etext, srodata, eroata, sdata, edata, sbss, ebss;
extern char boot_stack, boot_stack_top;

// 清零.bss段
void clear_bss() {
  for (char* i = &sbss; i < &ebss; i++) {
    *i = 0;
  }   
}

int main() {
  clear_bss();
  print_logo();
  shutdown();
  return 0;
}