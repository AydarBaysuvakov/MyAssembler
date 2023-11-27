#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "errors.h"
#include "colour.h"


Error_t ColouredPrintf(Colour colour, const char* format, ...)
    {
    assert(format != NULL);

    printf("\u001b[" "%d" "m", colour);

    va_list arg_p;
    va_start(arg_p, format);
    vprintf(format, arg_p);
    va_end(arg_p);

    printf("\u001b[0m");

    return Ok;
    }
