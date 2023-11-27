#ifndef COMPILER_H
#define COMPILER_H

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

Error_t CompilerCtor(Compiler *comp, const char* file_from, const char* file_to);

Error_t CompilerDtor(Compiler* comp);

Error_t DoCompilation(const char* file_from, const char* file_to);

Error_t TextToCode(Compiler *comp);

Error_t AddArgToCode(const int com, char* data, Compiler* comp);

char** SkipSpace(char** data);
char** SkipLetter(char** data);
char** SkipTillCollon(char** data);


bool IsCondJump(int cmd);
bool IsStkCmd(int cmd);
int GetOpcode(int cell);
int GetArgType(int cell);
bool ArgTypeIsData(int cmd);

#endif//COMPILER_H
