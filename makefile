# Variables
VERSION = 0.1.1
KERNELNAME = Arcxzs

AS = as
NASM = nasm
CC = gcc
LD = ld
GRUBISO = grub-mkrescue
GRUBISOO = ArxOS.iso
QEMU = qemu-system-i386
QEMU8664 = qemu-system-x86_64

UEFIFLAGS = -drive if=pflash,format=raw,readonly=on,file=/usr/share/qemu/edk2-x86_64-code.fd -cdrom ArxOS.iso

ASFLAGS = --32
NASMFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -O2 -fno-stack-protector -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld

TARGET = Arx.bin

# heap.o not included
OBJS = heap.o boot.o hw_io.o keyboard.o vga.o shell.o gdt.o idt.o interrupts.o printf.o kernel.o

# Default target
all: $(TARGET)

#test uefi
run-uefi:
# you may need to change this command to make it work becuase it uses a file on the disk that is differently placed in different distros.
	$(QEMU8664) $(UEFIFLAGS)

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

# Compile printf library
printf.o: lib/printf.c include/printf.h include/vga.h
	$(CC) $(CFLAGS) lib/printf.c -o printf.o

# Compile shell source
shell.o: src/shell.c include/shell.h include/vga.h include/keyboard.h
	$(CC) $(CFLAGS) src/shell.c -o shell.o

heap.o: src/heap.c include/vga.h
	$(CC) $(CFLAGS) src/heap.c -o heap.o

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

