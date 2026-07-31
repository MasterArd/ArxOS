[bits 32]
global idt_load
global gdt_flush
global irq0_stub
global irq1_stub

extern timer_handler
extern keyboard_handler

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; GDT Flush is for load GDT and reloading the segments
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

; IRQ0 timer stub
irq0_stub:
    pusha
    call timer_handler
    popa
    iretd

; IRQ1 keyboard stub
irq1_stub:
    pusha
    call keyboard_handler
    popa
    iretd
    
