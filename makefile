# Variables
AS = as
NASM = nasm
CC = gcc
LD = ld
GRUBISO = grub-mkrescue
GRUBISOO = ArxOS.iso
QEMU = qemu-system-i386
QEMU8664 = qemu-system-x86_64

ASFLAGS = --32
NASMFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -O2 -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld

TARGET = Arx.bin
OBJS = boot.o hw_io.o keyboard.o vga.o shell.o gdt.o idt.o interrupts.o kernel.o

# Default target
all: $(TARGET)

# Link the final binary
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Assemble boot.asm (GNU Assembler)
boot.o: boot.asm
	$(AS) $(ASFLAGS) boot.asm -o boot.o

# Assemble interrupt stubs (NASM)
interrupts.o: src/interrupts.asm
	$(NASM) $(NASMFLAGS) src/interrupts.asm -o interrupts.o

# Compile hardware I/O source
hw_io.o: src/hw_io.c include/hw_io.h
	$(CC) $(CFLAGS) src/hw_io.c -o hw_io.o

# Compile IDT & PIC source
idt.o: src/idt.c include/idt.h include/hw_io.h
	$(CC) $(CFLAGS) src/idt.c -o idt.o

# Compile GDT source
gdt.o: src/gdt.c include/gdt.h
	$(CC) $(CFLAGS) src/gdt.c -o gdt.o

# Compile keyboard source
keyboard.o: src/keyboard.c include/keyboard.h include/hw_io.h include/vga.h
	$(CC) $(CFLAGS) src/keyboard.c -o keyboard.o

# Compile VGA source
vga.o: src/vga.c include/vga.h
	$(CC) $(CFLAGS) src/vga.c -o vga.o

# Compile shell source
shell.o: src/shell.c include/shell.h include/vga.h include/keyboard.h
	$(CC) $(CFLAGS) src/shell.c -o shell.o

# Compile kernel entry point
kernel.o: src/kernel.c include/shell.h include/vga.h include/gdt.h include/idt.h
	$(CC) $(CFLAGS) src/kernel.c -o kernel.o

# Make ISO (Copies binary to iso/boot and runs grub-mkrescue)
iso: $(TARGET)
	mkdir -p iso/boot
	cp $(TARGET) iso/boot/$(TARGET)
	$(GRUBISO) -o $(GRUBISOO) iso

# Run kernel directly in QEMU (Multiboot test)
run: $(TARGET)
	$(QEMU) -kernel $(TARGET)

# Run ISO in QEMU x86_64
run-iso: iso
	$(QEMU8664) -cdrom $(GRUBISOO)

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET) $(GRUBISOO)

.PHONY: all run run-iso iso clean

