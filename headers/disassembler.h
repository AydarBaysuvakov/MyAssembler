#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

const char* const DEFAULT_TXT_FILENAME = "programs/program.txt";

struct Disassembler
    {
    FILE* file_to;

    unsigned char  *code;
    size_t cur_pos;
    size_t code_size;

    int labels[LABELS_COUNT];
    int labels_num;
    };

error_t DisassemblerCtor(Disassembler* disasm, const char* file_from, const char* file_to);

error_t DisassemblerDtor(Disassembler* disasm);

error_t DoDisassemblation(const char* file_from, const char* file_to);

error_t SearchLables(Disassembler *disasm);

error_t CodeToText(Disassembler *disasm);

error_t AddArg(Disassembler* disasm, const char* name);

int FindLable(Disassembler *disasm, int index);

#endif//DISASSEMBLER_H
