#include "shell.h"
#include "vga.h"
#include "printf.h"
#include "idt.h"
#include "gdt.h"
#include "hw_io.h"
#include "heap.h"

volatile uint32_t timer_ticks = 0;
void timer_handler(void) {
    timer_ticks++;
    outb(0x20, 0x20);
}

void kernel_main(void)
{   
    clear_screen();
    gdt_init();
    print_green("Starting gdt\n");
    idt_init();
    print_green("Starting idt\n");
    heap_init();
    print_green("Starting heap\n\n");
    print_green("ArxOS kernel loaded.\n");

    shell_run();
    while (1)
    {
        __asm__ volatile("cli; hlt");
    }
}
