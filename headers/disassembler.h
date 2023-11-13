#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

struct Disassembler
    {
    FILE* file_to;

    unsigned char *code;
    size_t cur_pos;
    size_t code_size;

    int labels[LABELS_COUNT];
    int labels_num;
    };

enum    {
        kNotFound = -1
        };

error_t DisassemblerCtor(Disassembler* disasm, const char* file_from, const char* file_to);

error_t DisassemblerDtor(Disassembler* disasm);

error_t Disassemble(const char* file_from, const char* file_to);

error_t SearchLables(Disassembler *disasm);

error_t CodeToText(Disassembler *disasm);

error_t AddArg(Disassembler* disasm, const char* name);

int FindLable(const Disassembler *disasm, int address);

bool IsCondJump(int cmd);

bool IsStkCmd(int cmd);

int GetOpcode(int cell);

int GetArgType(int cell);

#endif//DISASSEMBLER_H
