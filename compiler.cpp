#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/constants.h"
#include "usefullibs/oneginlib.h"
#include "headers/compiler.h"

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_BIN_FILENAME;

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
        perror("ERROR: cannot allocate memory"); // try
        return ALLOCATION_ERROR;
        }

    comp->file_to   = fopen(file_to,  "wb");
    if (comp->file_to == NULL)
        {
        perror("ERROR: cannot open file");
        return FILE_ERROR;
        }

    return OK;
    }

error_t CompilerDtor(Compiler* comp)
    {
    assert(comp != NULL);

    TextDtor(&comp->program);

    comp->cur_pos   = 0;
    comp->code_size = 0;

    free(comp->code);
    fclose(comp->file_to);

    return OK;
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

    return OK;
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
                return SYNTAX_ERROR;
                }
            }
        }

    #undef DEF_CMD
    #undef MAKE_COND_JUMP

    return OK;
    }

error_t AddArgToCode(const int cmd, char* data, Compiler* comp)
    {
    assert(data != NULL);
    assert(comp != NULL);

    const char is_cond_jump = (1 <= cmd && cmd <= 2) ? 0 : 1;

    SkipSpace(&data);

    if ((isdigit(data[0]) || data[0] == '-' || data[0] == '+') && cmd == command_push)
        {
        comp->code[comp->cur_pos] = cmd | CmdData;
        comp->cur_pos++;

        int arg = (int) (atof(data) * FIXED_POINT_MULTIPIER); // errors
        *(int*)(comp->code + comp->cur_pos) = arg;
        comp->cur_pos += sizeof(int); // typedef
        }
    else if (data[0] == '\'' && data[2] == '\'' && cmd == command_push)
        {
        comp->code[comp->cur_pos] = cmd | CmdData;
        comp->cur_pos++;

        *(int*)(comp->code + comp->cur_pos) = data[1] * FIXED_POINT_MULTIPIER;
        comp->cur_pos += sizeof(int);
        data += 3;
        }
    else if (data[0] == '[' && !is_cond_jump)
        {
        comp->code[comp->cur_pos] = cmd | CmdMemory;
        comp->cur_pos++;
        data++;

        *(int*)(comp->code + comp->cur_pos) = atoi(data);
        comp->cur_pos += sizeof(int);
        }
    else if (isalpha(data[0]))
        {
        int arg_found = 0;
        for (int iter = 0; iter < (is_cond_jump) ? comp->labels_num : comp->registers_num; iter++)
            {
            if (((is_cond_jump) ? comp->labels[iter].name : comp->registers[iter].name) == NULL)
                {
                break;
                }
            else if (!strncmp((is_cond_jump) ? comp->labels[iter].name : comp->registers[iter].name,\
            data, (is_cond_jump) ? comp->labels[iter].length : comp->registers[iter].length))
                {
                comp->code[comp->cur_pos] = cmd | CmdRegister;
                comp->cur_pos++;

                *(int*)(comp->code + comp->cur_pos) = (is_cond_jump) ? comp->labels[iter].index : comp->registers[iter].index;
                comp->cur_pos += sizeof(int);

                arg_found = 1;
                break;
                }
            }
        if (!arg_found)
            {
            int index = comp->registers_num;
            if (!is_cond_jump)
                {
                if (comp->registers_num == REGISTER_COUNT)
                    {
                    printf("SYNTAX ERROR: to many registers(available only 4, use memory)\n");
                    return SYNTAX_ERROR;
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

            *(int*)(comp->code + comp->cur_pos) = (is_cond_jump) ? -1 : comp->registers[index].index;
            comp->cur_pos += sizeof(int);
            }
        }
    else
        {
        printf("ERROR: %s is wrong command\n", data);
        return SYNTAX_ERROR;
        }

    SkipLetter(&data);
    SkipSpace(&data);

    if (*data != '\0')
        {
        printf("ERROR: %s is wrong command\n", data);
        return SYNTAX_ERROR;
        }

    return OK;
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
