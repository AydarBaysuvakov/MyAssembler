#ifndef SPU_H
#define SPU_H

const int REGISTER_COUNT = 4;

struct SPU
    {
    const char* file;

    Stack stk = {};

    int registers[REGISTER_COUNT];

    char *code;
    char *ip;
    size_t code_size;
    };

error_t SpuCtor(SPU* spu, const char* file);

error_t SpuDtor(SPU* spu);

error_t Run(const char* file);

#endif//SPU_H
