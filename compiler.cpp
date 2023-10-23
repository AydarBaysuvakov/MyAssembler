#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/constants.h"
#include "headers/oneginlib.h"
#include "headers/compiler.h"


int main(int argc, char *argv[])
    {
    if (argc < 2)
        {
        printf("File is not found");
        return FILE_ERROR;
        }
    else if (argc == 2)
        {
        const char* file_from = argv[1];
        const char* file_to   = OUTFILE;
        DoCompilation(file_from, file_to);
        }
    else if (argc > 2)
        {
        const char* file_from = argv[1];
        const char* file_to   = argv[2];
        DoCompilation(file_from, file_to);
        }
    }

error_t CompilerCtor(Compiler* comp, const char* file_from, const char* file_to)
    {
    assert(comp != NULL);
    assert(file_from != NULL);
    assert(file_to   != NULL);

    comp->program = {};
    TextCtor(&comp->program, file_from);

    comp->cur_pos   = 0;
    comp->code_size = comp->program.n_lines * (sizeof(int) + 1);

    comp->code = (char*) calloc(comp->code_size, sizeof(char));
    if (comp->code == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return ALLOCATION_ERROR;
        }

    comp->file_to   = fopen(file_to,  "wb");

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

    #define DEF_COM(name, com, args, ...)                                                               \
                    if (!strncmp(#name, *line, strlen(#name)))                                          \
                        {                                                                               \
                        if (!args)                                                                      \
                            {                                                                           \
                            comp->code[comp->cur_pos] = command_##name;                                 \
                            comp->cur_pos++;                                                            \
                            }                                                                           \
                        if (args)                                                                       \
                            {                                                                           \
                            AddArgToCode(com, *line + strlen(#name) + 1, comp->code, &comp->cur_pos);   \
                            }                                                                           \
                        }                                                                               \
                    else                                                                                \

    #define MAKE_COND_JUMP(name, com, ...)                                                              \
                    if (!strncmp(#name, *line, strlen(#name)))                                          \
                        {                                                                               \
                        AddJumpCond(com, *line + strlen(#name) + 1, comp);                              \
                        }                                                                               \
                    else                                                                                \

    for (char** line = comp->program.lines; (line - comp->program.lines) < comp->program.n_lines; line++)
        {
        #include "headers/dsl.h"
        {
        char* ch = strchr(*line, ':');
        if (ch == *line)
            {
            comp->labels[comp->labels_num].label = ++ch;
            int lenght = 0;
            while (!isspace(*ch) && *ch != '\0')
                {
                ch++;
                lenght++;
                }
            comp->labels[comp->labels_num].lenght = lenght;
            comp->labels[comp->labels_num].pos    = comp->cur_pos;
            comp->labels_num++;
            }
        else if (ch)
            {
            ch = *line;
            comp->labels[comp->labels_num].label = *line;
            int lenght = 0;
            while (*ch != ':')
                {
                ch++;
                lenght++;
                }
            comp->labels[comp->labels_num].lenght = lenght;
            comp->labels[comp->labels_num].pos    = comp->cur_pos;
            comp->labels_num++;
            }
        else if (**line)
            {
            printf("ERROR: %s is wrong command\n", *line);
            return SYNTAX_ERROR;
            }
        }
        }

    #undef DEF_COM
    #undef MAKE_JUMP_COND

    return OK;
    }

error_t AddArgToCode(const int com, char* data, char* code, size_t* cur_pos)
    {
    assert(2 >= com && com > 0);
    assert(data != NULL);
    assert(code != NULL);
    assert(cur_pos != NULL);

    SkipData(&data, SkipSpace);

    if (data[0] == 'r' && 'a' <= data[1] && data[1] <= 'd' && data[2] == 'x')
        {
        code[*cur_pos] = com | (1 << 6);
        (*cur_pos)++;

        code[*cur_pos] = data[1] - 'a';
        (*cur_pos)++;
        }
    else if (isdigit(data[0]))
        {
        code[*cur_pos] = com | (1 << 5);
        (*cur_pos)++;

        int arg = (int) (atof(data));
        int *intCode = (int*) (code + *cur_pos);
        *intCode = arg;
        *cur_pos += sizeof(int);
        }
    else
        {
        printf("ERROR: %s is wrong command\n", data);
        return SYNTAX_ERROR;
        }

    SkipData(&data, SkipLetter);
    SkipData(&data, SkipSpace);

    if (*data != '\0')
        {
        printf("here");
        printf("ERROR: %s is wrong command\n", data);
        return SYNTAX_ERROR;
        }

    return OK;
    }

error_t AddJumpCond(const int com, char* data, Compiler* comp)
    {
    assert(data != NULL);
    assert(comp != NULL);

    SkipData(&data, SkipSpace);

    Bool flag = False;

    for (int lab = 0; lab < comp->labels_num; lab++)
        {
        if (!strncmp(comp->labels[lab].label, data, comp->labels[lab].lenght))
            {
            comp->code[comp->cur_pos] = com | (1 << 5);
            comp->cur_pos++;

            comp->code[comp->cur_pos] = comp->labels[lab].pos;
            comp->cur_pos++;

            flag = True;
            break;
            }
        }

    if (!flag)
        {
        comp->code[comp->cur_pos] = com | (1 << 5);
        comp->cur_pos++;

        comp->code[comp->cur_pos] = -1;
        comp->cur_pos++;
        }

    SkipData(&data, SkipLetter);
    SkipData(&data, SkipSpace);

    if (*data != '\0')
        {
        printf("ERROR: %s is wrong command\n", data);
        return SYNTAX_ERROR;
        }

    return OK;
    }

error_t SkipData(char** data, Mode mode)
    {
    if (mode)
        {
        while (isspace(**data))
            {
            (*data)++;
            }
        return OK;
        }
    while (!isspace(**data) && **data != '\0')
            {
            (*data)++;
            }
    return OK;
    }
