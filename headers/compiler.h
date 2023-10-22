#ifndef COMPILER_H
#define COMPILER_H

const char* OUTFILE = "a.txt";
const int LABELS_COUNT = 20;

struct Label
    {
    char* label;
    int  lenght;
    int  pos;
    };

struct Compiler
    {
    FILE* file_from;
    FILE* file_to;

    Text program;
    Label labels[LABELS_COUNT];
    int labels_num = 0;

    char  *code;
    size_t cur_pos;
    size_t code_size;
    };

enum Mode
    {
    SkipSpace  = 1,
    SkipLetter = 0
    };

error_t CompilerCtor(Compiler *comp, const char* file_from, const char* file_to);

error_t CompilerDtor(Compiler* comp);

error_t DoCompilation(const char* file_from, const char* file_to);

error_t AddArgToCode(const int com, char* data, char* code, size_t *cur_pos);

size_t GetCodeSize(Text* program);

error_t SkipData(char** data, Mode mode);

error_t AddJumpCond(const int com, char* data, Compiler* comp);

#endif//COMPILER_H
