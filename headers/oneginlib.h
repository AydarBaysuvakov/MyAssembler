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

error_t TextCtor(Text *text, const char *file_name);

error_t TextDtor(Text *text);

error_t make_buf(Text *text, const char *file_name);

error_t fill_buf(char *buf, size_t buf_size, FILE *fp);

error_t lines_partition(Text *text);

error_t fill_lines(Text *text);

error_t text_to_file(Text* text, FILE* fp = stdout);

error_t print_text(Text* text, const char* file_name = NULL);

size_t file_size(FILE *fp);

size_t lines_count(const char *buf);

#endif //__IOTEXT__
