#ifndef __IOTEXT__
#define __IOTEXT__

struct Text
    {
    char * buf;
    size_t buf_size;
    char ** lines;
    size_t n_lines;
    };

const int EMPTY = 0;

Error_t TextCtor(Text *text, const char *file_name);

Error_t TextDtor(Text *text);

Error_t MakeBuf(Text *text, const char *file_name);

Error_t FillBuf(char *buf, size_t buf_size, FILE *fp);

Error_t LinesPartition(Text *text);

Error_t FillLines(Text *text);

Error_t TextToFile(Text* text, FILE* fp = stdout);

Error_t PrintText(Text* text, const char* file_name = NULL);

size_t FileSize(FILE *fp);

size_t LinesCount(const char *buf);

#endif //__IOTEXT__
