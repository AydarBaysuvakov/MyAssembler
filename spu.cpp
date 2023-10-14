#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "headers/constants.h"
#include "headers/oneginlib.h"
#include "headers/stack.h"
#include "headers/spu.h"


int main(int argc, char *argv[])
    {
    if (argc < 2)
        {
        printf("File is not found");
        return FILE_ERROR;
        }
    else
        {
        const char* file = argv[1];
        Run(file);
        }
    }

error_t SpuCtor(SPU* spu, const char* file)
    {
    assert(spu != NULL);
    assert(file != NULL);

    spu->file = file;

    StackCtor(&spu->stk);

    memset(spu->registers, 0, REGISTER_COUNT);

    FILE *fp = fopen(file, "rb");
    if (fp == nullptr)
        {
        perror("ERROR: cannot open file");
        return FILE_ERROR;
        }

    spu->code_size = file_size(fp);
    if (spu->code_size == -1)
        {
        perror("ERROR: fstat() func returned -1");
        return FILE_ERROR;
        }

    spu->code = (char*) calloc(spu->code_size, sizeof(char));
    if (spu->code == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        return ALLOCATION_ERROR;
        }

    fread(spu->code, sizeof(char), spu->code_size, fp);
    spu->ip = spu->code;

    if (fclose(fp) == EOF)
        {
        perror("ERROR: cannot close file");
        return FILE_ERROR;
        }

    return OK;
    }

error_t SpuDtor(SPU* spu)
    {
    assert(spu != NULL);

    StackDtor(&spu->stk);

    memset(spu->registers, 0, REGISTER_COUNT);

    spu->code_size = 0;

    spu->ip = nullptr;

    free(spu->code);

    return OK;
    }

error_t Run(const char* file)
    {
    assert(file != NULL);

    SPU spu = {};
    SpuCtor(&spu, file);

    #define DEF_COM(name, com, args, code, ...)                                                     \
                        case command_##name:                                                        \
                            code;                                                                   \
                            break;                                                                  \

    for (; (spu.ip - spu.code) < spu.code_size; spu.ip++)
        {
        int command = *spu.ip % (1 << 5);
        switch (command)
            {
            #include "headers/dsl.h"
            default:
                printf("ERROR: BAD CODE, error_command = %d\n", command);
                exit(1);
                break;
            }
        }

    #undef DEF_COM

    SpuDtor(&spu);

    return OK;
    }
