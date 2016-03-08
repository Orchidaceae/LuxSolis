.data
.align 2
.text

#enable_interrupts
.global enable_interrupts     #must be global, so it can be reached from mipslabwork.c
enable_interrupts:
  ei                          #enables interrupts mips instruction
  jr  $ra                     #jumps back
  nop                         #branch delay
