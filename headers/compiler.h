#ifndef COMPILER_H
#define COMPILER_H

// assembly language
// assembler

const char* const   DEFAULT_BIN_FILENAME  = "programs/out.bin";

struct Parametr
    {
    char* name;
    int  length;
    int  index;
    };

struct Compiler
    {
    FILE* file_to;

    Text program;
    Parametr labels[LABELS_COUNT];
    int labels_num;
    Parametr registers[REGISTER_COUNT];
    int registers_num;

    char  *code;
    size_t cur_pos;
    size_t code_size;
    };

error_t CompilerCtor(Compiler *comp, const char* file_from, const char* file_to);

error_t CompilerDtor(Compiler* comp);

error_t DoCompilation(const char* file_from, const char* file_to);

error_t TextToCode(Compiler *comp);

error_t AddArgToCode(const int com, char* data, Compiler* comp);

char** SkipSpace(char** data);
char** SkipLetter(char** data);
char** SkipTillCollon(char** data);

#endif//COMPILER_H
