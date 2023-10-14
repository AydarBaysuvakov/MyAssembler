#ifndef COMPILER_H
#define COMPILER_H

const char* OUTFILE = "a.txt";

struct Compiler
    {
    const char* file_from;
    const char* file_to;

    Text program;

    char  *code;
    size_t cur_pos;
    size_t code_size;
    };

error_t CompilerCtor(Compiler *comp, const char* file_from, const char* file_to);

error_t CompilerDtor(Compiler* comp);

error_t DoCompilation(const char* file_from, const char* file_to);

error_t AddArgToCode(const int com, const char* data, char* code, size_t *cur_pos);

size_t GetCodeSize(Text* program);

#endif//COMPILER_H
