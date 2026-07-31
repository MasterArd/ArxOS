#ifndef VGA_H
#define VGA_H

#include <stdarg.h>

void clear_screen(void);
void putchar(char c);
int print(const char *format, ...);
void backspace(void);
void print_green(const char *str);


#endif
