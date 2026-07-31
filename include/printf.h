#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list args);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, unsigned int size, const char *format, ...);
int vsprintf(char *buffer, const char *format, va_list args);
int vsnprintf(char *buffer, unsigned int size, const char *format, va_list args);

#endif
