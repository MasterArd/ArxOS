#include "shell.h"
#include "vga.h"
#include "printf.h"
#include "idt.h"
#include "gdt.h"
#include "hw_io.h"

volatile uint32_t timer_ticks = 0;
void timer_handler(void) {
    timer_ticks++;
    outb(0x20, 0x20);
}

void kernel_main(void)
{
    gdt_init();
    idt_init();
    clear_screen();
    printf("ArxOS kernel loaded.\n");
    shell_run();
    while (1)
    {
        __asm__ volatile("cli; hlt");
    }
}
