#include "headers/constants.h"
#include "headers/spu.h"

struct SPU
    {
    const char* file;

    Stack stk;

    int registers[4];

    char  *code;
    size_t cur_pos;
    size_t code_size;
    };
