/*
    I could not figure out how to do this myself... :(
    Thats why there are so much comments..............
*/


#include "vga.h"
#include "hw_io.h"
#include "printf.h"
#include <stdarg.h>
#include <stdint.h>

#define WIDTH 80
#define HEIGHT 25

// base memory address for VGA text mode buffer
static volatile uint16_t *vga = (uint16_t *)0xB8000;

// cursor state
static int row = 0;
static int col = 0;
static uint8_t color = 0x0F; // cight white text on black background
static uint8_t green_color = 0x0A;
// 0x0A is black back green front


// --- hardware cursor 

static void move_cursor(void)
{
    uint16_t pos = row * WIDTH + col;

    // send low byte of cursor position
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    // send high byte of cursor position
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// --- scrolling helper ---

static void scroll(void)
{
    // If we haven't reached the bottom, do nothing
    if (row < HEIGHT) {
        return;
    }

    // Move every line up by one row
    for (int r = 0; r < HEIGHT - 1; r++) {
        for (int c = 0; c < WIDTH; c++) {
            vga[r * WIDTH + c] = vga[(r + 1) * WIDTH + c];
        }
    }

    // Clear the bottom line
    for (int c = 0; c < WIDTH; c++) {
        vga[(HEIGHT - 1) * WIDTH + c] = ((uint16_t)color << 8) | ' ';
    }

    // Keep cursor on the last line
    row = HEIGHT - 1;
}

// --- Public VGA Functions ---

void clear_screen(void)
{
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            vga[r * WIDTH + c] = ((uint16_t)color << 8) | ' ';
        }
    }
    row = 0;
    col = 0;
    move_cursor();
}

void putchar(char c)
{
    // Handle Newline
    if (c == '\n') {
        row++;
        col = 0;
        scroll();
        move_cursor();
        return;
    }

    // Handle Tab
    if (c == '\t') {
        col = (col + 4) & ~3; // Align to next 4-space tab stop
        if (col >= WIDTH) {
            col = 0;
            row++;
            scroll();
        }
        move_cursor();
        return;
    }

    // Write character and attribute byte to memory
    uint16_t entry = ((uint16_t)color << 8) | (uint8_t)c;
    vga[row * WIDTH + col] = entry;
    col++;

    // Wrap to next line if end of row reached
    if (col >= WIDTH) {
        col = 0;
        row++;
    }

    scroll();
    move_cursor();
}

int print(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

void putchar_green(char c)
{
    // Handle Newline
    if (c == '\n') {
        row++;
        col = 0;
        scroll();
        move_cursor();
        return;
    }

    // Handle Tab
    if (c == '\t') {
        col = (col + 4) & ~3; // Align to next 4-space tab stop
        if (col >= WIDTH) {
            col = 0;
            row++;
            scroll();
        }
        move_cursor();
        return;
    }

    // Write character and attribute byte to memory
    uint16_t entry = ((uint16_t)green_color << 8) | (uint8_t)c;
    vga[row * WIDTH + col] = entry;
    col++;

    // Wrap to next line if end of row reached
    if (col >= WIDTH) {
        col = 0;
        row++;
    }

    scroll();
    move_cursor();
}
void print_green(const char *str)
{
    while (*str) {
        putchar_green(*str);
        str++;
    }
}

void backspace(void)
{
    if (col > 0) {
        col--;
        vga[row * WIDTH + col] = ((uint16_t)color << 8) | ' ';
        move_cursor();
    } 
    // Handle backspacing to previous line
    else if (row > 0) {
        row--;
        col = WIDTH - 1;
        vga[row * WIDTH + col] = ((uint16_t)color << 8) | ' ';
        move_cursor();
    }
}

void set_color(uint8_t fg, uint8_t bg)
{
    color = (bg << 4) | (fg & 0x0F);
}