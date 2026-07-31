.section .multiboot, "a"
.align 4

.long 0x1BADB002          
.long 0x00                
.long -(0x1BADB002 + 0x00)

.section .bss
.align 16
stack_bottom:
.skip 16384                /*16kb stack */
stack_top:

.section .text
.global _start
.extern kernel_main

_start:
    mov $stack_top, %esp  /* init stack */
    call kernel_main

hang:
    hlt
    jmp hang

    