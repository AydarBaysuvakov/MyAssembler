#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include "headers/constants.h"
#include "MyLib/mylib.h"
#include "headers/spu.h"

// ISA -- instruction set architecture

int main(int argc, char *argv[])
    {
    const char* file = nullptr;

    if (argc < 2)
        {
        printf("Incorrect args number");
        return FileError;
        }
    else
        {
        file = argv[1];
        }

    SpuRun(file);
    return 0;
    }

Error_t MySpuCtor(SPU* spu,
                  const char* exe_file,
                  const char* name,
                  const unsigned line,
                  const char* file,
                  const char* func)
    {
    assert(spu != NULL);
    assert(file != NULL);

    spu->exe_file = exe_file;

    StackCtor(&spu->stk);

    memset(spu->registers, 0, REGISTER_COUNT);
    memset(spu->memory   , 0, MEMORY_SIZE);

    FILE *fp = fopen(exe_file, "rb");
    if (fp == nullptr)
        {
        perror("ERROR: cannot open file");
        return FileError;
        }

    struct stat sb = {0};
    int fd = fileno(fp);

    if (fstat(fd, &sb) == -1)
        {
        perror("ERROR: fstat() func returned -1");
        fclose(fp);
        return FileError;
        }
    spu->code_size = sb.st_size;

    spu->code = (unsigned char*) calloc(spu->code_size, sizeof(char));
    if (spu->code == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        fclose(fp);
        return AllocationError;
        }

    fread(spu->code, sizeof(char), spu->code_size, fp);
    if (ferror(fp))
        {
        perror("Cannot read file\n");
        }
    spu->ip = 0;

    fclose(fp);

    spu->name = name;
    spu->line = line;
    spu->file = file;
    spu->func = func;

    if (LogFileInit(&spu->logfile, "logfile", name, "html") == FileError)
        {
        perror("ERROR: cannot open logfile");
        free(spu->code);
        return FileError;
        }

    return Ok;
    }

Error_t SpuDtor(SPU* spu)
    {
    assert(spu != NULL);

    StackDtor(&spu->stk);

    memset(spu->registers, 0, REGISTER_COUNT);
    memset(spu->memory   , 0, MEMORY_SIZE);

    spu->code_size = 0;

    spu->ip = 0;

    free(spu->code);

    fclose(spu->logfile);

    return Ok;
    }

Error_t SpuRun(const char* file)
    {
    assert(file != NULL);

    SPU spu = {};
    SpuCtor(&spu, file);

    #define DEF_CMD(name, cmd, args, code, ...)                                                     \
                        case command_##name:                                                        \
                            code;                                                                   \
                            break;                                                                  \

    #define MAKE_COND_JUMP(name, cmd, code, ...)                                                    \
                        case cond_##name:                                                        \
                            code;                                                                   \
                            break;                                                                  \

    for (; spu.ip < spu.code_size; spu.ip++)
        {
        int command = spu.code[spu.ip] & CmdField;
        switch (command)
            {
            #include "headers/dsl.h"
            default:
                SpuDump(&spu);
                printf("ERROR: BAD CODE, error_command = %d, %d, %d\n", command, spu.code[spu.ip], spu.ip);
                return BadCode;
                break;
            }
        //SpuDump(&spu);
        }

    #undef DEF_CMD
    #undef MAKE_COND_JUMP

    SpuDtor(&spu);

    return Ok;
    }

Error_t MySpuDump(const SPU *spu, const char* name, const unsigned line,
                  const char* file, const char* func)
    {

    fprintf(spu->logfile, "SPU[%p] '%s' from %s(%u) %s()\n", spu, spu->name, spu->file, spu->line, spu->func);
    fprintf(spu->logfile, "\tcalled like '%s' from %s(%u) %s()\n",     name,      file,      line,      func);
    fprintf(spu->logfile, "\trunning file: \"%s\"\n", spu->exe_file);
    fprintf(spu->logfile, "\t{\n");

    fprintf(spu->logfile, "\tCode:\n\t\t");
    for (size_t iter = 0; iter < spu->code_size; iter++)
        {
        fprintf(spu->logfile, "[%2x]", spu->code[iter]);
        }

    fprintf(spu->logfile, "\n\t\t");
    for (size_t iter = 0; iter < spu->ip; iter++)
        {
        fprintf(spu->logfile, "    ");
        }
    fprintf(spu->logfile, " ^^ ");
    fprintf(spu->logfile, "\n\tinstruction pointer = %d\n\n", spu->ip);

    fprintf(spu->logfile, "\tRegisters\n");
    fprintf(spu->logfile, "\t\t rax: %d\n", spu->registers[0]);
    fprintf(spu->logfile, "\t\t rbx: %d\n", spu->registers[1]);
    fprintf(spu->logfile, "\t\t rcx: %d\n", spu->registers[2]);
    fprintf(spu->logfile, "\t\t rdx: %d\n", spu->registers[3]);
    fprintf(spu->logfile, "\t\t rex: %d\n", spu->registers[4]);
    fprintf(spu->logfile, "\t\t rfx: %d\n", spu->registers[5]);
    fprintf(spu->logfile, "\t\t rgx: %d\n", spu->registers[6]);
    fprintf(spu->logfile, "\t\t rhx: %d\n", spu->registers[7]);

    fprintf(spu->logfile, "\n\tMemory:");
    for (int i = 0; i < MEMORY_DUMP_LENGTH; ++i)
        {
        fprintf(spu->logfile, "\n\t\t");
        for (int j = 0; j < MEMORY_DUMP_WIDTH; ++j)
            {
            fprintf(spu->logfile, "[%3d ]", spu->memory[i * MEMORY_DUMP_WIDTH + j]);
            }
        }

    fprintf(spu->logfile, "\n\n\tStack:\n\t\t{\n");
    for (size_t iter = 0; iter < spu->stk.capacity; iter++)
        {
        fprintf(spu->logfile, (spu->stk.data[iter] != POISON) ? "\t\t*" : "\t\t ");
        fprintf(spu->logfile, "[%lu] = ", iter);
        fprintf(spu->logfile, (spu->stk.data[iter] == POISON) ? "POISON\n" : "%d\n", spu->stk.data[iter]);
        }
    fprintf(spu->logfile, "\t\t}\n");

    fprintf(spu->logfile, "\t}\n\n");

    return Ok;
    }

Error_t MemDump(SPU *spu)
    {
    for (int i = 0; i < MEMORY_DUMP_LENGTH; ++i)
        {
        printf("\n");
        for (int j = 0; j < MEMORY_DUMP_WIDTH; ++j)
            {
            ColouredPrintf(BLACK, (isspace(spu->memory[i * MEMORY_DUMP_WIDTH + j]) || !spu->memory[i * MEMORY_DUMP_WIDTH + j]) ? "." : "%c", spu->memory[i * MEMORY_DUMP_WIDTH + j] / FIXED_POINT_MULTIPIER);
            }
        }
    printf("\n");

    return Ok;
    }
