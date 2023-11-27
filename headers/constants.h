#ifndef CONSTANTS_H
#define CONSTANTS_H

const int FIXED_POINT_MULTIPIER = 100;
const int ARG_TYPE_BORDER       = 5;
const int REGISTER_COUNT        = 12;
const int LABELS_COUNT          = 1000;
const int MEMORY_SIZE           = 3200;
const int MEMORY_DUMP_WIDTH     = 80;
const int MEMORY_DUMP_LENGTH    = 40;

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

#endif//CONSTANTS_H
