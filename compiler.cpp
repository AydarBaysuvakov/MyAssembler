#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/constants.h"
#include "usefullibs/oneginlib.h"
#include "headers/compiler.h"

const char* const   DEFAULT_BIN_FILENAME  = "programs/out.bin";

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_BIN_FILENAME;

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

    DoCompilation(file_from, file_to);
    return 0;
    }

error_t CompilerCtor(Compiler* comp, const char* file_from, const char* file_to)
    {
    assert(comp != NULL);
    assert(file_from != NULL);
    assert(file_to   != NULL);

    comp->program = {};
    TextCtor(&comp->program, file_from);

    comp->labels_num    = 0;
    comp->registers_num = 0;

    comp->cur_pos   = 0;
    comp->code_size = comp->program.n_lines * (sizeof(int) + 1);

    comp->code = (char*) calloc(comp->code_size, sizeof(char));
    if (comp->code == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    comp->file_to   = fopen(file_to,  "wb");
    if (comp->file_to == NULL)
        {
        perror("ERROR: cannot open file");
        return FileError;
        }

    return Ok;
    }

error_t CompilerDtor(Compiler* comp)
    {
    assert(comp != NULL);

    TextDtor(&comp->program);

    comp->cur_pos   = 0;
    comp->code_size = 0;

    free(comp->code);
    fclose(comp->file_to);

    return Ok;
    }

error_t DoCompilation(const char* file_from, const char* file_to)
    {
    assert(file_from != NULL);
    assert(file_to   != NULL);

    Compiler comp = {};
    CompilerCtor(&comp, file_from, file_to);

    TextToCode(&comp);
    TextToCode(&comp);

    fwrite(comp.code, sizeof(char), comp.cur_pos, comp.file_to);

    CompilerDtor(&comp);

    return Ok;
    }

error_t TextToCode(Compiler *comp)
    {
    assert(comp != NULL);

    comp->cur_pos = 0;

    #define DEF_CMD(name, cmd, args, ...)                                                               \
                    else if (!strncmp(#name, comand, strlen(#name)))                                    \
                        {                                                                               \
                        if (!args)                                                                      \
                            {                                                                           \
                            comp->code[comp->cur_pos] = command_##name;                                 \
                            comp->cur_pos++;                                                            \
                            }                                                                           \
                        if (args)                                                                       \
                            {                                                                           \
                            AddArgToCode(cmd, comand + strlen(#name) + 1, comp);                        \
                            }                                                                           \
                        }                                                                                         \

    #define MAKE_COND_JUMP(name, cmd, ...)                                                              \
                    else if (!strncmp(#name, comand, strlen(#name)))                                    \
                        {                                                                               \
                        AddArgToCode(cmd, comand + strlen(#name) + 1, comp);                            \
                        }

    for (int line = 0; line < comp->program.n_lines; line++)
        {
        char* comand = comp->program.lines[line];
        SkipSpace(&comand);

        if (0);
        #include "headers/dsl.h"
        else
            {
            char* ch = strchr(comand, ':');
            if (ch == comand)
                {
                comp->labels[comp->labels_num].name   = ++ch;
                SkipLetter(&ch);
                comp->labels[comp->labels_num].length = ch - comand;
                comp->labels[comp->labels_num].index  = comp->cur_pos;
                comp->labels_num++;
                }
            else if (ch)
                {
                ch = comand;
                comp->labels[comp->labels_num].name   = comand;
                SkipTillCollon(&ch);
                comp->labels[comp->labels_num].length = ch - comand;
                comp->labels[comp->labels_num].index  = comp->cur_pos;
                comp->labels_num++;
                }
            else if (*comand)
                {
                printf("ERROR: %s is wrong command code_pos = %ld\n", comand, comp->cur_pos);
                return SyntaxError;
                }
            }
        }

    #undef DEF_CMD
    #undef MAKE_COND_JUMP

    return Ok;
    }

error_t AddArgToCode(const int cmd, char* data, Compiler* comp)
    {
    assert(data != NULL);
    assert(comp != NULL);

    SkipSpace(&data);

    if ((isdigit(data[0]) || data[0] == '-' || data[0] == '+') && ArgTypeIsData(cmd))
        {
        comp->code[comp->cur_pos] = cmd | CmdData;
        comp->cur_pos++;

        int arg = (int) (atof(data) * FIXED_POINT_MULTIPIER);
        *(int*)(comp->code + comp->cur_pos) = arg;
        comp->cur_pos += sizeof(int);
        }
    else if (data[0] == '\'' && data[2] == '\'' && ArgTypeIsData(cmd))
        {
        comp->code[comp->cur_pos] = cmd | CmdData;
        comp->cur_pos++;

        *(int*)(comp->code + comp->cur_pos) = data[1] * FIXED_POINT_MULTIPIER;
        comp->cur_pos += sizeof(int);
        data += 3;
        }
    else if (data[0] == '[' && !IsCondJump(cmd))
        {
        comp->code[comp->cur_pos] = cmd | CmdMemory;
        comp->cur_pos++;
        data++;

        *(int*)(comp->code + comp->cur_pos) = atoi(data);
        comp->cur_pos += sizeof(int);
        }
    else if (isalpha(data[0]))
        {
        bool arg_found = false;
        for (int i = 0; i < (IsCondJump(cmd)) ? comp->labels_num : comp->registers_num; i++)
            {
            if (((IsCondJump(cmd)) ? comp->labels[i].name : comp->registers[i].name) == NULL)
                {
                break;
                }
            else if (!strncmp((IsCondJump(cmd)) ? comp->labels[i].name : comp->registers[i].name,\
            data, (IsCondJump(cmd)) ? comp->labels[i].length : comp->registers[i].length))
                {
                comp->code[comp->cur_pos] = cmd | CmdRegister;
                comp->cur_pos++;

                *(int*)(comp->code + comp->cur_pos) = (IsCondJump(cmd)) ? comp->labels[i].index : comp->registers[i].index;
                comp->cur_pos += sizeof(int);

                arg_found = true;
                break;
                }
            }
        if (!arg_found)
            {
            int index = comp->registers_num;
            if (!IsCondJump(cmd))
                {
                if (comp->registers_num == REGISTER_COUNT)
                    {
                    printf("SYNTAX ERROR: to many registers(available only 8, use memory)\n");
                    return SyntaxError;
                    }
                comp->registers[index].name    = data;
                char* word = data;
                SkipLetter(&word);
                comp->registers[index].length  = word - data;
                comp->registers[index].index   = index;
                comp->registers_num++;
                }

            comp->code[comp->cur_pos] = cmd | CmdRegister;
            comp->cur_pos++;

            *(int*)(comp->code + comp->cur_pos) = (IsCondJump(cmd)) ? -1 : comp->registers[index].index;
            comp->cur_pos += sizeof(int);
            }
        }
    else
        {
        printf("ERROR: %s is wrong command\n", data);
        return SyntaxError;
        }

    SkipLetter(&data);
    SkipSpace(&data);

    if (*data != '\0')
        {
        printf("ERROR: %s is wrong command\n", data);
        return SyntaxError;
        }

    return Ok;
    }

char** SkipSpace(char** data)
    {
    assert(data != NULL);

    while (isspace(**data))
            {
            (*data)++;
            }

    return data;
    }

char** SkipLetter(char** data)
    {
    assert(data != NULL);

    while (!isspace(**data) && **data != '\0')
            {
            (*data)++;
            }

    return data;
    }

char** SkipTillCollon(char** data)
    {
    assert(data != NULL);

    while (**data != ':' && **data != '\0')
            {
            (*data)++;
            }

    return data;
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

bool ArgTypeIsData(int cmd)
    {
    return cmd == command_push || cmd == command_set;
    }
