#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include "headers/constants.h"
#include "MyLib/errors.h"
#include "headers/disassembler.h"

const char* const DEFAULT_TXT_FILENAME = "programs/program.txt";

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_TXT_FILENAME;

    if (argc < 2)
        {
        printf("Incorrect args number");
        return FileError;
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

    Disassemble(file_from, file_to);
    return 0;
    }

Error_t DisassemblerCtor(Disassembler* disasm, const char* file_from, const char* file_to)
    {
    assert(disasm    != NULL);
    assert(file_from != NULL);
    assert(file_to   != NULL);

    FILE *fp = fopen(file_from, "rb");
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
    disasm->code_size = sb.st_size;

    disasm->code = (unsigned char*) calloc(disasm->code_size, sizeof(char));
    if (disasm->code == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        fclose(fp);
        return AllocationError;
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
        free(disasm->code);
        return FileError;
        }

    return Ok;
    }

Error_t DisassemblerDtor(Disassembler* disasm)
    {
    assert(disasm != NULL);

    disasm->cur_pos    = 0;
    disasm->code_size  = 0;
    disasm->labels_num = 0;

    free(disasm->code);
    fclose(disasm->file_to);

    return Ok;
    }

Error_t Disassemble(const char* file_from, const char* file_to)
    {
    assert(file_from != NULL);
    assert(file_to   != NULL);

    Disassembler disasm = {};
    DisassemblerCtor(&disasm, file_from, file_to);

    SearchLables(&disasm);
    CodeToText(&disasm);

    DisassemblerDtor(&disasm);

    return Ok;
    }

Error_t SearchLables(Disassembler *disasm)
    {
    assert(disasm != NULL);

    disasm->cur_pos = 0;

    for (;disasm->cur_pos < disasm->code_size; disasm->cur_pos++)
        {
        int cmd = GetOpcode(disasm->code[disasm->cur_pos]);

        if (IsCondJump(cmd))
            {
            disasm->cur_pos++;
            int arg = FindLable(disasm, *((int*) &disasm->code[disasm->cur_pos]));
            if (arg == kNotFound)
                {
                disasm->labels[disasm->labels_num] = *((int*) &disasm->code[disasm->cur_pos]);
                disasm->labels_num++;
                disasm->cur_pos += sizeof(int) - 1;
                }
            }
        else if (IsStkCmd(cmd))
            {
            disasm->cur_pos += sizeof(int);
            }
        }

    return Ok;
    }

Error_t CodeToText(Disassembler *disasm)
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
        if (lable != kNotFound)
            {
            fprintf(disasm->file_to, "lable%i:\n", lable);
            }
        char cmd = GetOpcode(disasm->code[disasm->cur_pos]);
        switch (cmd)
            {
            #include "headers/dsl.h"
            default:
                printf("ERROR: BAD CODE, error_command = %d, %d, %ld\n", cmd, disasm->code[disasm->cur_pos], disasm->cur_pos);
                return BadCode;
            }
        }

    #undef DEF_CMD
    #undef MAKE_COND_JUMP

    return Ok;
    }

Error_t AddArg(Disassembler* disasm, const char* name)
    {
    assert(disasm != NULL);
    assert(name != NULL);

    char arg_type = GetArgType(disasm->code[disasm->cur_pos]);
    char cmd      =  GetOpcode(disasm->code[disasm->cur_pos]);

    fprintf(disasm->file_to, "%s ", name);
    disasm->cur_pos++;

    if (IsCondJump(cmd))
        {
        int lable = FindLable(disasm, *((int*) &disasm->code[disasm->cur_pos]));
        fprintf(disasm->file_to, "lable%i\n", lable);
        }
    else
        switch (arg_type)
            {
            case DataType:
                {
                float arg = (float) *(int*)(disasm->code + disasm->cur_pos) / FIXED_POINT_MULTIPIER;
                fprintf(disasm->file_to, "%f\n", arg);
                break;
                }
            case RegisterType:
                {
                fprintf(disasm->file_to, "reg%i\n", disasm->code[disasm->cur_pos]);
                break;
                }
            case MemoryType:
                {
                int arg = *(int*)(disasm->code + disasm->cur_pos);
                fprintf(disasm->file_to, "[%d]\n", arg);
                break;
                }
            }

    disasm->cur_pos += sizeof(int) - 1;
    return Ok;
    }

int FindLable(const Disassembler *disasm, int address)
    {
    assert(disasm != NULL);
    assert(address >= 0);

    for (int i = 0; i < disasm->labels_num; i++)
        {
        if (disasm->labels[i] == address)
            {
            return i;
            }
        }

    return kNotFound;
    }

bool IsCondJump(int cmd)
    {
    return cond_jmp <= cmd && cmd <= cond_call;
    }

bool IsStkCmd(int cmd)
    {
    return command_push <= cmd && cmd <= command_pop;
    }

int GetOpcode(int cell)
    {
    return cell & CmdField;
    }


int GetArgType(int cell)
    {
    return cell >> ARG_TYPE_BORDER;
    }
