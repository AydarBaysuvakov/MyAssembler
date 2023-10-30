#ifndef CONSTANTS_H
#define CONSTANTS_H

const int FIXED_POINT_MULTIPIER = 100;
const int ARG_TYPE_BORDER       = 5;
const int REGISTER_COUNT        = 4;
const int LABELS_COUNT          = 20;
const int MEMORY_SIZE           = 128;
const int MEMORY_DUMP_WIDTH     = 16;
const int MEMORY_DUMP_LENGTH    = 8;

enum ArgTypes
    {
    DataType       =  1 << 0,
    RegisterType   =  1 << 1,
    MemoryType     =  1 << 2,
    CmdData        =  1 << 5,
    CmdRegister    =  1 << 6,
    CmdMemory      =  1 << 7,
    CmdField       =  31
    };

#define DEF_CMD(name, num, ...)         command_##name = num,
#define MAKE_COND_JUMP(name, num, ...)  cond_##name = num,

enum Commands
    {
    #include "dsl.h"
    last_command
    };

#undef DEF_CMD
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
    EMPTY_STACK           = 7,
    BAD_CODE              = 8,
    EXIT                  = 9
    };

#endif//CONSTANTS_H
