# ArxOS

ArxOS is a small 32-bit x86 operating system kernel built from scratch in C and assembly. It boots in protected mode using a Multiboot-compatible entry point, initializes the GDT and IDT, sets up a basic VGA console, and provides an interactive shell for simple commands.

Status: early-stage kernel. It is intended for experimentation and learning, not as a production OS.

---

## Features

- 32-bit x86 protected mode kernel
- Multiboot-compatible boot flow via GRUB
- GDT and IDT setup
- IRQ0 timer handling and IRQ1 keyboard handling
- VGA text-mode console output
- Simple PS/2 keyboard input loop
- Minimal built-in shell with command execution
- Heap initialization and debug dump support

---

## Project Layout

```text
.
├── boot.asm              Boot entry and Multiboot header
├── linker.ld             Linker script for the kernel layout
├── makefile              Build and run commands
├── README.md             Project overview and usage notes
├── include/              Public headers
│   ├── gdt.h
│   ├── heap.h
│   ├── hw_io.h
│   ├── idt.h
│   ├── keyboard.h
│   ├── printf.h
│   ├── shell.h
│   └── vga.h
├── lib/
│   └── printf.c          Minimal printf implementation
├── src/
│   ├── gdt.c             Global descriptor table setup
│   ├── heap.c            Basic heap initialization and dump utility
│   ├── hw_io.c           I/O port helpers for x86 access
│   ├── idt.c             Interrupt descriptor table setup
│   ├── interrupts.asm    Interrupt entry stubs
│   ├── kernel.c          Kernel entry point and main startup flow
│   ├── keyboard.c        Keyboard interrupt/input handling
│   ├── shell.c           Command parser and shell loop
│   ├── vga.c             VGA text-mode driver
│   └── loader.c          Additional loader-related code
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg GRUB config for the ISO boot image
└── LICENSE               GPL license information
```

---

## Quick Start

### Requirements

- gcc with 32-bit support
- GNU assembler (`as`)
- GNU linker with ELF32 support
- NASM for assembly interrupt stubs
- GRUB utilities (`grub-mkrescue`) for creating an ISO image
- QEMU for testing (`qemu-system-i386` or `qemu-system-x86_64`)

### Build the kernel

```bash
make
```

This produces the kernel binary `Arx.bin`.

### Build a bootable ISO

```bash
make iso
```

### Run directly in QEMU

```bash
make run
```

### Run the ISO in QEMU

```bash
make run-iso
```

### Clean build artifacts

```bash
make clean
```

---

## Shell Behavior

When the kernel boots, it prints startup messages and starts the shell. The login prompt expects the password:

```text
arc
```

Once unlocked, the shell prompt appears as:

```text
Arc:kernel>
```

### Built-in commands

```text
help       Show available commands
clear      Clear the display
about      Display kernel details
ping       Echo "pong"
heap-dump  Dump heap state if available
```

The shell is intentionally minimal and is used to demonstrate interrupt-driven keyboard input, command parsing, and kernel output.

---

## Runtime Overview

1. `boot.asm` sets up the Multiboot header and transfers control to the kernel.
2. `kernel_main()` initializes the display, GDT, IDT, heap, and shell.
3. Keyboard interrupts are processed through the PS/2 path.
4. The shell awaits user input and dispatches commands.
5. The timer interrupt increments `timer_ticks` and is used as a basic timing hook.

---

## Known Limitations

- BIOS/legacy boot flow only; no UEFI support
- No paging or virtual memory management
- No filesystem implementation
- No multiprocessing or scheduler
- Minimal command set and kernel utilities
- Educational code with no full OS runtime guarantees

---

## Learning Goals

This project is useful for learning:

- Multiboot kernel entry and boot process
- x86 protected mode fundamentals
- Global descriptor tables and interrupt descriptor tables
- PIC remapping and IRQ handling
- VGA text-mode output
- Assembly-to-C integration
- Basic keyboard input and shell design

---

## License

This project is licensed under the GNU GPL v3.0. See the LICENSE file for details.

---

## Notes

The repository includes some naming inconsistencies, such as the internal kernel label `Arcxzs` in the Makefile versus the project name `ArxOS`. The core OS is still early in development, but the current build flow and shell already work for a simple x86 hobby kernel demonstration.
