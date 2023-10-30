#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/constants.h"
#include "usefullibs/oneginlib.h"
#include "headers/disassembler.h"

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_TXT_FILENAME;

    if (argc < 2)
        {
        printf("Incorrect args number");
        return FILE_ERROR;
        }
    else if (argc == 2)
        {
        file_from = argv[1];
        }
    else if (argc > 2)
        {
        file_from = argv[1];
        file_to   = argv[2];
        }

    DoDisassemblation(file_from, file_to);
    return 0;
    }

error_t DisassemblerCtor(Disassembler* disasm, const char* file_from, const char* file_to)
    {
    assert(disasm != NULL);
    assert(file_from != NULL);
    assert(file_to   != NULL);

    FILE *fp = fopen(file_from, "rb");
    if (fp == nullptr)
        {
        perror("ERROR: cannot open file");
        return FILE_ERROR;
        }

    disasm->code_size = file_size(fp);
    if (disasm->code_size == -1)
        {
        perror("ERROR: fstat() func returned -1");
        fclose(fp);
        return FILE_ERROR;
        }

    disasm->code = (unsigned char*) calloc(disasm->code_size, sizeof(char));
    if (disasm->code == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        fclose(fp);
        return ALLOCATION_ERROR;
        }

    fread(disasm->code, sizeof(char), disasm->code_size, fp);
    if (ferror(fp))
        {
        perror("Cannot read file\n");
        }
    disasm->cur_pos  = 0;

    disasm->labels_num = 0;

    fclose(fp);

    disasm->file_to = fopen(file_to, "w");
    if (disasm->file_to == nullptr)
        {
        perror("ERROR: cannot open file");
        return FILE_ERROR;
        }

    return OK;
    }

error_t DisassemblerDtor(Disassembler* disasm)
    {
    assert(disasm != NULL);

    disasm->cur_pos    = 0;
    disasm->code_size  = 0;
    disasm->labels_num = 0;

    free(disasm->code);
    fclose(disasm->file_to);

    return OK;
    }

error_t DoDisassemblation(const char* file_from, const char* file_to)
    {
    assert(file_from != NULL);
    assert(file_to   != NULL);

    Disassembler disasm = {};
    DisassemblerCtor(&disasm, file_from, file_to);

    SearchLables(&disasm);
    CodeToText(&disasm);

    DisassemblerDtor(&disasm);

    return OK;
    }

error_t SearchLables(Disassembler *disasm)
    {
    assert(disasm != NULL);

    disasm->cur_pos = 0;

    for (;disasm->cur_pos < disasm->code_size; disasm->cur_pos++)
        {
        int cmd = disasm->code[disasm->cur_pos] & CmdField;
        if (16 <= cmd && cmd <= 23)
            {
            disasm->cur_pos++;
            int pos = FindLable(disasm, *((int*) &disasm->code[disasm->cur_pos]));
            if (pos == -1)
                {
                disasm->labels[disasm->labels_num] = *((int*) &disasm->code[disasm->cur_pos]);
                disasm->labels_num++;
                disasm->cur_pos += sizeof(int) - 1;
                }
            }
        else if (1 <= cmd && cmd <= 2)
            {
            disasm->cur_pos += sizeof(int);
            }
        }

    return OK;
    }

error_t CodeToText(Disassembler *disasm)
    {
    assert(disasm != NULL);

    disasm->cur_pos = 0;

    #define DEF_CMD(name, cmd, args, ...)                                                           \
                        case command_##name:                                                        \
                            if (!args)                                                              \
                                {                                                                   \
                                fprintf(disasm->file_to, "%s\n", #name);                             \
                                }                                                                   \
                            else                                                                    \
                                {                                                                   \
                                AddArg(disasm, #name);                                               \
                                }                                                                   \
                            break;                                                                  \

    #define MAKE_COND_JUMP(name, cmd, ...)                                                          \
                        case cond_##name:                                                           \
                            AddArg(disasm, #name);                                                   \
                            break;

    for (; disasm->cur_pos < disasm->code_size; disasm->cur_pos++)
        {
        int lable = FindLable(disasm, disasm->cur_pos);
        if (lable != -1)
            {
            fprintf(disasm->file_to, "lable%i:\n", lable);
            }
        char command = disasm->code[disasm->cur_pos] & CmdField;
        switch (command)
            {
            #include "headers/dsl.h"
            default:
                printf("ERROR: BAD CODE, error_command = %d, %d, %ld\n", command, disasm->code[disasm->cur_pos], disasm->cur_pos);
                return BAD_CODE;
            }
        }

    #undef DEF_CMD
    #undef MAKE_COND_JUMP

    return OK;
    }

error_t AddArg(Disassembler* disasm, const char* name)
    {
    assert(disasm != NULL);
    assert(name != NULL);

    unsigned char cell     = disasm->code[disasm->cur_pos];
    char arg_type = cell >> ARG_TYPE_BORDER;
    char cmd      = cell & CmdField;

    const char is_cond_jump = (1 <= cmd && cmd <= 2) ? 0 : 1;

    if (is_cond_jump)
        {
        fprintf(disasm->file_to, "%s ", name);
        disasm->cur_pos++;

        int lable = FindLable(disasm, *((int*) &disasm->code[disasm->cur_pos]));
        fprintf(disasm->file_to, "lable%i\n", lable);
        disasm->cur_pos += sizeof(int) - 1;
        }
    else if (arg_type == DataType)
        {
        fprintf(disasm->file_to, "%s ", name);
        disasm->cur_pos++;

        float arg = (float) *(int*)(disasm->code + disasm->cur_pos) / FIXED_POINT_MULTIPIER;
        fprintf(disasm->file_to, "%f\n", arg);
        disasm->cur_pos += sizeof(int) - 1;
        }
    else if (arg_type == RegisterType)
        {
        fprintf(disasm->file_to, "%s ", name);
        disasm->cur_pos++;

        fprintf(disasm->file_to, "reg%i\n", disasm->code[disasm->cur_pos]);
        disasm->cur_pos += sizeof(int) - 1;
        }
    else if (arg_type == MemoryType)
        {
        fprintf(disasm->file_to, "%s ", name);
        disasm->cur_pos++;

        int arg = *(int*)(disasm->code + disasm->cur_pos);
        fprintf(disasm->file_to, "[%d]\n", arg);
        disasm->cur_pos += sizeof(int) - 1;
        }

    return OK;
    }

int FindLable(Disassembler *disasm, int index)
    {
    assert(disasm != NULL);

    for (int iter = 0; iter < disasm->labels_num; iter++)
        {
        if (disasm->labels[iter] == index)
            {
            return iter;
            }
        }

    return -1;
    }
