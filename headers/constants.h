#ifndef CONSTANTS_H
#define CONSTANTS_H

const int MUL_COEFF = 100;

#define DEF_COM(name, num, ...)         command_##name = num,
#define MAKE_COND_JUMP(name, num, ...)  cond_##name = num,

enum Commands
    {
    #include "dsl.h"
    last_command
    };

#undef DEF_COM
#undef MAKE_COND_JUMP

enum error_t
    {
    OK                    = 0,
    FILE_ERROR            = 1,
    ALLOCATION_ERROR      = 2,
    BUFFER_OVERFLOW_ERROR = 3,
    CALCULATION_ERROR     = 4,
    HASH_NOT_COMPARE      = 5,
    SYNTAX_ERROR          = 6,
    EMPTY_STACK           = 7
    };

#endif//CONSTANTS_H
