#include "../include/printf.h"
#include "../include/sbi.h"
#include <stdint.h>

// 用来进行数字打印的转换
static char digits[] = "0123456789abcdef";

void printstring(char *s) {
    while (*s) {
        console_putchar(*s++);
    }
}

static void printint(int xx, int base, int sign) {
    char buf[16];
    int i;
    unsigned x;

    if (sign && (sign = xx < 0)) {
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;
    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign) {
        buf[i++] = '-';
    }
    // 反向输出字符
    while (--i >= 0) {
        console_putchar(buf[i]);
    }
}

static void printptr(uint64_t addr) {
    int i;
    console_putchar('0');
    console_putchar('x');
    for (i = 0; i < sizeof(uint64_t) * 2; i++, addr <<= 4) {
        console_putchar(digits[addr >> (sizeof(uint64_t) * 8 - 4)]);
    }
}

void printf(char *fmt, ...) {
    va_list arg;
    int i, c;
    char *s;

    if (fmt == 0) {
        panic("null fmt");
    }
    va_start(arg, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            console_putchar(c);
            continue;
        }
        // 跳过%
        c = fmt[++i] & 0xff;
        if (c == 0) {
            break;
        }

        switch (c) {
        case 'd':
            printint(va_arg(arg, int), 10, 1);
            break;
        case 'x':
            printint(va_arg(arg, int), 16, 0);
            break;
        case 'p':
            printptr(va_arg(arg, uint64_t));
            break;
        case 's':
            s = va_arg(arg, char *);
            if (s == 0) {
                s = "(null)";
            }
            printstring(s);
            break;
        case '%':
            console_putchar(c);
            break;
        default:
            // Unknown % sequence. Print it to draw attention.
            console_putchar('%');
            console_putchar(c);
            break;
        }
    }
}

// void backtrace() {
//   uint64_t *fp = (uint64_t *)r_fp();
//   uint64_t *bottom =

// }
void panic(char *s) {
    printf("panic: %s\n", s);
    // TODO: backtrace
    while (1)
        ;
}

void print_logo() {
    printf("\n\n\n\n");
    printf("ooooo      ooo   .oooooo.     .oooooo.   ooooooooo.   oooooooooooo   .oooooo.    .oooooo..o\n");
    printf("`888b.     `8'  d8P'  `Y8b   d8P'  `Y8b  `888   `Y88. `888'     `8  d8P'  `Y8b  d8P'    `Y8\n");
    printf("8 `88b.    8  888          888      888  888   .d88'  888         888      888 Y88bo.      \n");
    printf("8   `88b.  8  888          888      888  888ooo88P'   888oooo8    888      888  `\"Y8888o. \n");
    printf("8     `88b.8  888          888      888  888`88b.     888    \"    888      888      `\"Y88b\n");
    printf("8       `888  `88b    ooo  `88b    d88'  888  `88b.   888       o `88b    d88' oo     .d8P \n");
    printf("o8o        `8   `Y8bood8P'   `Y8bood8P'  o888o  o888o o888ooooood8  `Y8bood8P'  8"
           "88888P'  \n");
    printf("\nwelcome to ncore\n\n");
}