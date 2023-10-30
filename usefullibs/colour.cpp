#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "colour.h"

void ColouredPrintf(Colour colour, FILE* fp, const char* format, ...)
    {
    assert(format != NULL);

    fprintf(fp, "\u001b[" "%d" "m", colour);

    va_list arg_p;
    va_start(arg_p, format);
    vfprintf(fp, format, arg_p);
    va_end(arg_p);

    fprintf(fp, "\u001b[0m");
    }
