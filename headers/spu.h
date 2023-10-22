#ifndef SPU_H
#define SPU_H

#define SPU_DEFN_ARGS const char*, const unsigned, const char*, const char*
#define SPU_PASS_ARGS __LINE__, __FILE__, __FUNCTION__
#define SpuCtor(spu, exe)   MySpuCtor((spu), (exe), #spu, SPU_PASS_ARGS)
#define SpuDump(spu)        MySpuDump((spu),        #spu, SPU_PASS_ARGS)

const int REGISTER_COUNT = 4;

struct SPU
    {
    const char* exe_file;

    Stack stk = {};

    int registers[REGISTER_COUNT];

    char *code;
    char *ip;
    size_t code_size;

    const char  *name;
    const char  *file;
    const char  *func;
    unsigned     line;

    FILE *logfile;
    };

error_t MySpuCtor(SPU* spu, const char* file, SPU_DEFN_ARGS);

error_t SpuDtor(SPU* spu);

error_t Run(const char* file);

error_t MySpuDump(const SPU *spu, SPU_DEFN_ARGS);

error_t SpuLogFileInit(SPU *spu, const char* name);

#endif//SPU_H
