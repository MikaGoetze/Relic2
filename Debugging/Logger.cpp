//
// Created by mikag on 9/30/2018.
//

#include <iostream>
#include "Logger.h"
#include <cstdarg>

void Logger::Log(const char *format, ...)
{
    const char *traverse;
    int i;
    char *s;

    va_list arg;
    va_start(arg, format);

    for (traverse = format; *traverse != '\0'; traverse++)
    {
        while (*traverse != '%' && *traverse != '\0')
        {
            putchar(*traverse);
            traverse++;
        }

        if(*traverse == '\0') break;

        traverse++;

        switch (*traverse)
        {
            case 'c' :
                i = va_arg(arg, int);
                putchar(i);
                break;

            case 'i':
            case 'd' :
                i = va_arg(arg, int);
                if (i < 0)
                {
                    i = -i;
                    putchar('-');
                }
                printf("%s", Convert(i, 10));
                break;

            case 'o':
                i = va_arg(arg, unsigned int);
                printf("%s", Convert(i, 8));
                break;

            case 's':
                s = va_arg(arg, char *);
                printf("%s", s);
                break;

            case 'x':
                i = va_arg(arg, unsigned int);
                printf("%s", Convert(i, 16));
                break;
        }
    }

    va_end(arg);
    printf("\n");
}

char * Logger::Convert(unsigned int num, int base)
{
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num % base];
        num /= base;
    } while (num != 0);

    return (ptr);
}
