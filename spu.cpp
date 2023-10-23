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

error_t MySpuCtor(SPU* spu, const char* exe_file, const char* name, const unsigned line, const char* file, const char* func)
    {
    assert(spu != NULL);
    assert(file != NULL);

    spu->exe_file = exe_file;

    StackCtor(&spu->stk);
    StackCtor(&spu->return_codes);

    memset(spu->registers, 0, REGISTER_COUNT);

    FILE *fp = fopen(exe_file, "rb");
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
    spu->ip = 0;

    if (fclose(fp) == EOF)
        {
        perror("ERROR: cannot close file");
        return FILE_ERROR;
        }

    spu->name = name;
    spu->line = line;
    spu->file = file;
    spu->func = func;

    if (SpuLogFileInit(spu, name) == FILE_ERROR)
        {
        perror("ERROR: cannot open logfile");
        return FILE_ERROR;
        }

    return OK;
    }

error_t SpuDtor(SPU* spu)
    {
    assert(spu != NULL);

    StackDtor(&spu->stk);
    StackDtor(&spu->return_codes);

    memset(spu->registers, 0, REGISTER_COUNT);

    spu->code_size = 0;

    spu->ip = 0;

    free(spu->code);

    fclose(spu->logfile);

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

    #define MAKE_COND_JUMP(name, com, code, ...)                                                    \
                        case cond_##name:                                                        \
                            code;                                                                   \
                            break;                                                                  \

    for (; spu.ip < spu.code_size; spu.ip++)
        {
        int command = spu.code[spu.ip] % (1 << 5);
        switch (command)
            {
            #include "headers/dsl.h"
            default:
                SpuDump(&spu);
                printf("ERROR: BAD CODE, error_command = %d\n", command);
                exit(1);
                break;
            }
        }

    #undef DEF_COM

    #undef MAKE_JUMP_COND

    SpuDtor(&spu);

    return OK;
    }

error_t MySpuDump(const SPU *spu, const char* name, const unsigned line, const char* file, const char* func)
    {

    fprintf(spu->logfile, "SPU[%p] '%s' from %s(%u) %s()\n", spu, spu->name, spu->file, spu->line, spu->func);
    fprintf(spu->logfile, "\tcalled like '%s' from %s(%u) %s()\n",     name,      file,      line,      func);
    fprintf(spu->logfile, "\trunning file: \"%s\"\n", spu->exe_file);
    fprintf(spu->logfile, "\t{\n");
    fprintf(spu->logfile, "\t rax: %d\n", spu->registers[0]);
    fprintf(spu->logfile, "\t rbx: %d\n", spu->registers[1]);
    fprintf(spu->logfile, "\t rcx: %d\n", spu->registers[2]);
    fprintf(spu->logfile, "\t rdx: %d\n", spu->registers[3]);

    fprintf(spu->logfile, "\tStack:\n");
    fprintf(spu->logfile, "\t\t{}\n");

    fprintf(spu->logfile, "\tCode:");

    for (size_t iter = 0; iter < spu->code_size; iter++)
        {
        fprintf(spu->logfile, " [%x]", spu->code[iter]);
        }
    fprintf(spu->logfile, "\n%d\n\t     ", spu->ip);
    for (size_t iter = 0; iter < spu->ip; iter++)
        {
        fprintf(spu->logfile, "    ");
        }
    fprintf(spu->logfile, "  ^  \n");

    fprintf(spu->logfile, "\t}\n\n");

    return OK;
    }

error_t SpuLogFileInit(SPU *spu, const char* name)
    {
    char file_name[LOG_FILE_MAX_NAME_LENGHT] = "logfiles/logfile(";
    strncat(file_name,  name   , LOG_FILE_NAME_LENGHT);
    strncat(file_name, ").html", LOG_FILE_NAME_LENGHT);

    spu->logfile = fopen(file_name, "w");
    if (spu->logfile == NULL)
        {
        return FILE_ERROR;
        }

    return OK;
    }
