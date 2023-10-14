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

    comp->file_from = file_from;
    comp->file_to   = file_to;

    comp->program = {};
    text_ctor(&comp->program, file_from);

    comp->cur_pos   = 0;
    comp->code_size = GetCodeSize(&comp->program);

    comp->code = (char*) calloc(comp->code_size, sizeof(char));
    if (comp->code == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return ALLOCATION_ERROR;
        }

    return OK;
    }

error_t CompilerDtor(Compiler* comp)
    {
    assert(comp != NULL);

    text_dtor(&comp->program);

    comp->cur_pos   = 0;
    comp->code_size = 0;

    free(comp->code);

    return OK;
    }

error_t DoCompilation(const char* file_from, const char* file_to)
    {
    assert(file_from != NULL);
    assert(file_to   != NULL);

    Compiler comp = {};
    CompilerCtor(&comp, file_from, file_to);

    #define DEF_COM(name, com, args, ...)                                                           \
                    if (!strncmp(#name, *line, strlen(#name)))                                      \
                        {                                                                           \
                        if (!args)                                                                  \
                            {                                                                       \
                            comp.code[comp.cur_pos] = command_##name;                               \
                            comp.cur_pos++;                                                         \
                            }                                                                       \
                        if (args)                                                                   \
                            {                                                                       \
                            AddArgToCode(com, *line + strlen(#name) + 1, comp.code, &comp.cur_pos); \
                            }                                                                       \
                        }                                                                           \
                    else                                                                            \

    for (char** line = comp.program.lines; (line - comp.program.lines) < comp.program.n_lines; line++)
        {
        if (NULL)
            {}
        #include "headers/dsl.h"
        {}
        }

    #undef DEF_COM


    FILE *file_to_p = fopen(file_to, "wb");
    fwrite(comp.code, sizeof(char), comp.code_size, file_to_p);

    CompilerDtor(&comp);

    return OK;
    }

error_t AddArgToCode(const int com, const char* data, char* code, size_t *cur_pos)
    {
    assert(2 >= com && com > 0);
    assert(data != NULL);
    assert(code != NULL);
    assert(cur_pos != NULL);

    while (isspace(*data))
        {
        data++;
        }

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
        printf("ERROR: %s is wrong command, not compiled\n", data);
        return SYNTAX_ERROR;
        }

    while (!isspace(*data) && *data != '\0')
        {
        data++;
        }

    while (isspace(*data))
        {
        data++;
        }

    if (*data != '\0')
        {
        printf("ERROR: %s is wrong command, skipped\n", data);
        return SYNTAX_ERROR;
        }

    return OK;
    }

size_t GetCodeSize(Text* program)
    {
    assert(program != NULL);

    int code_size = 0;

    for (char** line = program->lines; (line - program->lines) < program->n_lines; line++)
        {
        if (!strncmp("push", *line, 4) || !strncmp("pop", *line, 3))
            {
            code_size += sizeof(int);
            }
        code_size++;
        }

    return code_size;
    }
