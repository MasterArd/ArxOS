#include "printf.h"
#include "vga.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct printf_state {
    char *buffer;
    unsigned int size;
    unsigned int pos;
    int count;
    bool to_buffer;
};

static void output_char(struct printf_state *state, char c)
{
    if (state->to_buffer) {
        if (state->pos + 1 < state->size) {
            state->buffer[state->pos] = c;
        }
        state->pos++;
    } else {
        putchar(c);
    }
    state->count++;
}

static void output_string(struct printf_state *state, const char *str, int width, char pad, bool left_align)
{
    int len = 0;
    const char *p = str;
    while (*p++) {
        len++;
    }

    int pad_count = width > len ? width - len : 0;
    if (!left_align) {
        for (int i = 0; i < pad_count; i++) {
            output_char(state, pad);
        }
    }

    while (*str) {
        output_char(state, *str++);
    }

    if (left_align) {
        for (int i = 0; i < pad_count; i++) {
            output_char(state, ' ');
        }
    }
}

static void output_unsigned(struct printf_state *state,
                             unsigned long value,
                             int base,
                             bool uppercase,
                             int width,
                             char pad,
                             bool left_align,
                             const char *prefix)
{
    char number[32];
    int length = 0;

    if (value == 0) {
        number[length++] = '0';
    } else {
        while (value != 0) {
            unsigned int digit = value % base;
            number[length++] = digit < 10
                ? (char)('0' + digit)
                : (char)((uppercase ? 'A' : 'a') + digit - 10);
            value /= base;
        }
    }

    int prefix_len = 0;
    if (prefix) {
        while (prefix[prefix_len]) {
            prefix_len++;
        }
    }

    int total_len = length + prefix_len;
    int pad_count = width > total_len ? width - total_len : 0;

    if (!left_align && pad == ' ') {
        for (int i = 0; i < pad_count; i++) {
            output_char(state, ' ');
        }
    }

    if (prefix) {
        for (int i = 0; i < prefix_len; i++) {
            output_char(state, prefix[i]);
        }
    }

    if (!left_align && pad == '0') {
        for (int i = 0; i < pad_count; i++) {
            output_char(state, '0');
        }
    }

    for (int i = length - 1; i >= 0; i--) {
        output_char(state, number[i]);
    }

    if (left_align) {
        for (int i = 0; i < pad_count; i++) {
            output_char(state, ' ');
        }
    }
}

static void output_signed(struct printf_state *state,
                          long value,
                          int width,
                          char pad,
                          bool left_align)
{
    unsigned long abs_value;
    if (value < 0) {
        output_char(state, '-');
        abs_value = (unsigned long)(-value);
        if (width > 0) {
            width--;
        }
    } else {
        abs_value = (unsigned long)value;
    }
    output_unsigned(state, abs_value, 10, false, width, pad, left_align, NULL);
}

static int vformat(struct printf_state *state, const char *format, va_list args)
{
    while (*format) {
        if (*format != '%') {
            output_char(state, *format++);
            continue;
        }

        format++;
        if (*format == '%') {
            output_char(state, '%');
            format++;
            continue;
        }

        bool left_align = false;
        bool alternate = false;
        char pad = ' ';
        int width = 0;

        while (*format == '-' || *format == '0' || *format == '#') {
            if (*format == '-') {
                left_align = true;
            } else if (*format == '0' && !left_align) {
                pad = '0';
            } else if (*format == '#') {
                alternate = true;
            }
            format++;
        }

        if (*format == '*') {
            width = va_arg(args, int);
            if (width < 0) {
                left_align = true;
                width = -width;
            }
            format++;
        } else {
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format++ - '0');
            }
        }

        int length_modifier = 0;
        if (*format == 'l') {
            length_modifier = 1;
            format++;
            if (*format == 'l') {
                length_modifier = 2;
                format++;
            }
        }

        char specifier = *format++;
        switch (specifier) {
            case 'c': {
                char c = (char)va_arg(args, int);
                output_char(state, c);
                break;
            }

            case 's': {
                const char *str = va_arg(args, const char *);
                if (!str) {
                    str = "(null)";
                }
                output_string(state, str, width, pad, left_align);
                break;
            }

            case 'd':
            case 'i': {
                long value = length_modifier ? va_arg(args, long) : va_arg(args, int);
                output_signed(state, value, width, pad, left_align);
                break;
            }

            case 'u': {
                unsigned long value = length_modifier ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
                output_unsigned(state, value, 10, false, width, pad, left_align, NULL);
                break;
            }

            case 'x': {
                unsigned long value = length_modifier ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
                const char *prefix = alternate ? "0x" : NULL;
                output_unsigned(state, value, 16, false, width, pad, left_align, prefix);
                break;
            }

            case 'X': {
                unsigned long value = length_modifier ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
                const char *prefix = alternate ? "0X" : NULL;
                output_unsigned(state, value, 16, true, width, pad, left_align, prefix);
                break;
            }

            case 'p': {
                void *ptr = va_arg(args, void *);
                output_unsigned(state, (unsigned long)ptr, 16, false, width == 0 ? (int)(sizeof(void *) * 2) : width, '0', left_align, "0x");
                break;
            }

            default:
                output_char(state, '%');
                if (specifier) {
                    output_char(state, specifier);
                }
                break;
        }
    }

    return state->count;
}

int vsnprintf(char *buffer, unsigned int size, const char *format, va_list args)
{
    struct printf_state state;
    state.buffer = buffer;
    state.size = size;
    state.pos = 0;
    state.count = 0;
    state.to_buffer = true;

    int written = vformat(&state, format, args);
    if (size > 0) {
        unsigned int null_pos = state.pos < size ? state.pos : size - 1;
        buffer[null_pos] = '\0';
    }

    return written;
}

int snprintf(char *buffer, unsigned int size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer, size, format, args);
    va_end(args);
    return written;
}

int vsprintf(char *buffer, const char *format, va_list args)
{
    return vsnprintf(buffer, 0xFFFFFFFFu, format, args);
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int written = vsprintf(buffer, format, args);
    va_end(args);
    return written;
}

int vprintf(const char *format, va_list args)
{
    struct printf_state state;
    state.buffer = NULL;
    state.size = 0;
    state.pos = 0;
    state.count = 0;
    state.to_buffer = false;

    return vformat(&state, format, args);
}

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int written = vprintf(format, args);
    va_end(args);
    return written;
}
