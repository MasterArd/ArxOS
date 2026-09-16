#include "printf.h"
#include "vga.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct printf_state
{
    char *buffer;
    unsigned int size;
    unsigned int pos;
    int count;
    bool to_buffer;
};

static void output_char(struct printf_state *state, char c)
{
    if (state->to_buffer)
    {
        if (state->pos < state->size - 1)
        {
            state->buffer[state->pos] = c;
        }
        state->pos++;
    }
    else
    {
        putchar(c);
    }
    state->count++;
}

static void output_string(struct printf_state *state, const char *str, int width, char pad, bool left_align)
{
    int len = 0;
    const char *p = str;
    while (*p++)
    {
        len++;
    }

    int pad_count = width > len ? width - len : 0;
    if (!left_align)
    {
        for (int i = 0; i < pad_count; i++)
        {
            output_char(state, pad);
        }
    }

    while (*str)
    {
        output_char(state, *str++);
    }

    if (left_align)
    {
        for (int i = 0; i < pad_count; i++)
        {
            output_char(state, ' ');
        }
    }
}

unsigned long long __udivmoddi4(unsigned long long dividend,
                                unsigned long long divisor,
                                unsigned long long *remainder)
{
    unsigned long long quotient = 0;
    unsigned long long rem = 0;

    for (int bit = 63; bit >= 0; bit--)
    {
        rem = (rem << 1) | ((dividend >> bit) & 1ULL);
        if (rem >= divisor)
        {
            rem -= divisor;
            quotient |= 1ULL << bit;
        }
    }

    if (remainder)
    {
        *remainder = rem;
    }

    return quotient;
}

static void output_unsigned(struct printf_state *state,
                            unsigned long long value,
                            int base,
                            bool uppercase,
                            int width,
                            char pad,
                            bool left_align,
                            const char *prefix)
{
    char number[32];
    int length = 0;

    // Anything that actually fits in 32 bits - which on this -m32 target is
    // every %lu, %p, %u, %x/%X you have, "ll" or not - takes native machine
    // division. The hardware does this in one instruction, no runtime helper
    // involved.
    //
    // Only a genuine >32-bit magnitude falls through to the branch below,
    // which does the division itself with a bit-shift algorithm instead of
    // the % and / operators. Those operators on an `unsigned long long`
    // lower to calls to __udivdi3/__umoddi3 - NOT to __udivmoddi4 above,
    // despite the name similarity; that function is never invoked by the
    // compiler for a bare % or /, it's only a symbol name that happens to
    // match libgcc's internal helper. Whether __udivdi3/__umoddi3 exist in
    // this build depends entirely on what your linker pulls in, and if it
    // doesn't, or if it pulls in a libgcc object that *also* defines
    // __udivmoddi4, you get either an undefined reference or a symbol
    // collision - and either way, this is not something a printf
    // implementation should be gambling on.
    if (value == 0)
    {
        number[length++] = '0';
    }
    else if (value <= 0xFFFFFFFFULL)
    {
        unsigned long v = (unsigned long)value;
        unsigned long b = (unsigned long)base;
        while (v != 0)
        {
            unsigned int digit = (unsigned int)(v % b);
            number[length++] = digit < 10
                                   ? (char)('0' + digit)
                                   : (char)((uppercase ? 'A' : 'a') + digit - 10);
            v /= b;
        }
    }
    else
    {
        while (value != 0)
        {
            unsigned long long remainder;
            value = __udivmoddi4(value, (unsigned long long)base, &remainder);
            unsigned int digit = (unsigned int)remainder;
            number[length++] = digit < 10
                                   ? (char)('0' + digit)
                                   : (char)((uppercase ? 'A' : 'a') + digit - 10);
        }
    }

    int prefix_len = 0;
    if (prefix)
    {
        while (prefix[prefix_len])
        {
            prefix_len++;
        }
    }

    int total_len = length + prefix_len;
    int pad_count = width > total_len ? width - total_len : 0;

    if (!left_align && pad == ' ')
    {
        for (int i = 0; i < pad_count; i++)
        {
            output_char(state, ' ');
        }
    }

    if (prefix)
    {
        for (int i = 0; i < prefix_len; i++)
        {
            output_char(state, prefix[i]);
        }
    }

    if (!left_align && pad == '0')
    {
        for (int i = 0; i < pad_count; i++)
        {
            output_char(state, '0');
        }
    }

    for (int i = length - 1; i >= 0; i--)
    {
        output_char(state, number[i]);
    }

    if (left_align)
    {
        for (int i = 0; i < pad_count; i++)
        {
            output_char(state, ' ');
        }
    }
}

static void output_signed(struct printf_state *state,
                          long long value,
                          int width,
                          char pad,
                          bool left_align)
{
    unsigned long long abs_value;
    const char *prefix = NULL;

    if (value < 0)
    {
        prefix = "-";
        abs_value = (unsigned long long)(0 - (unsigned long long)value);
    }
    else
    {
        abs_value = (unsigned long long)value;
    }

    output_unsigned(state, abs_value, 10, false, width, pad, left_align, prefix);
}

static int vformat(struct printf_state *state, const char *format, va_list args)
{
    while (*format)
    {
        if (*format != '%')
        {
            output_char(state, *format++);
            continue;
        }

        format++;
        if (*format == '%')
        {
            output_char(state, '%');
            format++;
            continue;
        }

        bool left_align = false;
        bool alternate = false;
        char pad = ' ';
        int width = 0;

        while (*format == '-' || *format == '0' || *format == '#')
        {
            if (*format == '-')
            {
                left_align = true;
            }
            else if (*format == '0' && !left_align)
            {
                pad = '0';
            }
            else if (*format == '#')
            {
                alternate = true;
            }
            format++;
        }

        if (*format == '*')
        {
            width = va_arg(args, int);
            if (width < 0)
            {
                left_align = true;
                width = -width;
            }
            format++;
        }
        else
        {
            while (*format >= '0' && *format <= '9')
            {
                width = width * 10 + (*format++ - '0');
            }
        }

        int length_modifier = 0;
        if (*format == 'l')
        {
            length_modifier = 1;
            format++;
            if (*format == 'l')
            {
                length_modifier = 2;
                format++;
            }
        }

        char specifier = *format++;
        switch (specifier)
        {
        case 'c':
        {
            char c = (char)va_arg(args, int);
            char tmp[2] = {c, '\0'};
            output_string(state, tmp, width, pad, left_align);
            break;
        }

        case 's':
        {
            const char *str = va_arg(args, const char *);
            if (!str)
            {
                str = "(null)";
            }
            output_string(state, str, width, pad, left_align);
            break;
        }

        case 'd':
        case 'i':
        {
            long long value;
            if (length_modifier == 2)
            {
                value = va_arg(args, long long);
            }
            else if (length_modifier == 1)
            {
                value = va_arg(args, long);
            }
            else
            {
                value = va_arg(args, int);
            }
            output_signed(state, value, width, pad, left_align);
            break;
        }

        case 'u':
        {
            if (length_modifier == 2)
            {
                unsigned long long value = va_arg(args, unsigned long long);
                output_unsigned(state, value, 10, false, width, pad, left_align, NULL);
            }
            else if (length_modifier == 1)
            {
                unsigned long value = va_arg(args, unsigned long);
                output_unsigned(state, value, 10, false, width, pad, left_align, NULL);
            }
            else
            {
                unsigned int value = va_arg(args, unsigned int);
                output_unsigned(state, value, 10, false, width, pad, left_align, NULL);
            }
            break;
        }

        case 'x':
        {
            if (length_modifier == 2)
            {
                unsigned long long value = va_arg(args, unsigned long long);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, false, width, pad, left_align, prefix);
            }
            else if (length_modifier == 1)
            {
                unsigned long value = va_arg(args, unsigned long);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, false, width, pad, left_align, prefix);
            }
            else
            {
                unsigned int value = va_arg(args, unsigned int);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, false, width, pad, left_align, prefix);
            }
            break;
        }

        case 'X':
        {
            if (length_modifier == 2)
            {
                unsigned long long value = va_arg(args, unsigned long long);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, true, width, pad, left_align, prefix);
            }
            else if (length_modifier == 1)
            {
                unsigned long value = va_arg(args, unsigned long);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, true, width, pad, left_align, prefix);
            }
            else
            {
                unsigned int value = va_arg(args, unsigned int);
                const char *prefix = (alternate && value != 0) ? "0x" : NULL;
                output_unsigned(state, value, 16, true, width, pad, left_align, prefix);
            }
            break;
        }

        case 'p':
        {
            void *ptr = va_arg(args, void *);
            output_unsigned(state, (unsigned long long)(uintptr_t)ptr, 16, false, width, pad, left_align, "0x");
            break;
        }

        default:
            output_char(state, '%');
            if (specifier)
            {
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
    if (size > 0)
    {
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