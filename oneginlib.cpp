#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "headers/constants.h"
#include "headers/oneginlib.h"

error_t text_ctor(Text *text, const char *file_name)
    {
    assert(text != NULL);

    make_buf(text, file_name);
    lines_partition(text);

    return OK;
    }

error_t text_dtor(Text *text)
    {
    assert(text != NULL);

    free(text->buf);
    free(text->lines);

    text->buf_size = EMPTY;
    text->n_lines  = EMPTY;

    return OK;
    }

error_t make_buf(Text *text, const char *file_name)
    {
    assert(text      != NULL);
    assert(file_name != NULL);

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
        {
        perror("ERROR: cannot open file");
        return FILE_ERROR;
        }

    text->buf_size = file_size(fp);
    if (text->buf_size == -1)
        {
        perror("ERROR: fstat() func returned -1");
        return FILE_ERROR;
        }

    text->buf = (char*) malloc((text->buf_size + 1) * sizeof(char));
    if (text->buf == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        return ALLOCATION_ERROR;
        }

    if (fill_buf(text->buf, text->buf_size, fp))
        {
        perror("ERROR: buffer overflow");
        return BUFFER_OVERFLOW_ERROR;
        }

    if (fclose(fp) == EOF)
        {
        perror("ERROR: cannot close file");
        return FILE_ERROR;
        }

    return OK;
    }

error_t fill_buf(char *buf, size_t buf_size, FILE *fp)
    {
    assert(fp  != NULL);
    assert(buf != NULL);

    fread(buf, sizeof(*buf), buf_size, fp);

    return OK;
    }

error_t lines_partition(Text *text)
    {
    assert(text      != NULL);
    assert(text->buf != NULL);

    text->n_lines = lines_count(text->buf) + 1;

    text->lines = (char**) malloc((text->n_lines + 1) * sizeof(char*));
    if (text->lines == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return ALLOCATION_ERROR;
        }

    if (fill_lines(text))
        {
        perror("ERROR: buffer overflow");
        return BUFFER_OVERFLOW_ERROR;
        }

    return OK;
    }

error_t fill_lines(Text *text)
    {
    assert(text        != NULL);
    assert(text->buf   != NULL);
    assert(text->lines != NULL);

    char **line = text->lines;
    char *buf   = text->buf;
    *line++ = buf;

    for (; *buf != '\0'; buf++)
        {
        if (buf - text->buf > text->buf_size)
            {
            return BUFFER_OVERFLOW_ERROR;
            }
        if (line - text->lines > text->n_lines)
            {
            return BUFFER_OVERFLOW_ERROR;
            }
        if (*buf == '\n')
            {
            *buf = '\0';
            *line++ = buf + 1;
            }
        if (*buf == ';')
            {
            *buf = '\0';
            }
        }
    *line = nullptr;

    return OK;
    }

error_t text_to_file(Text* text, FILE* fp)
    {
    assert(text != NULL);
    assert(fp   != NULL);

    char** line = text->lines;

    for (; *line != NULL; line++)
        {
        fprintf(fp, "%s \n", *line);
        }

    return OK;
    }

error_t print_text(Text* text, const char* file_name)
    {
    assert(text        != NULL);
    assert(text->buf   != NULL);
    assert(text->lines != NULL);

    if (file_name == NULL)
        {
        text_to_file(text);
        return OK;
        }

    FILE *fp = fopen(file_name, "w");
    text_to_file(text, fp);
    fclose(fp);

    return OK;
    }

size_t file_size(FILE *fp)
    {
    assert(fp != NULL);

    struct stat sb = {0};
    int fd = fileno(fp);

    if (fstat(fd, &sb) == -1)
        {
        return -1;
        }

    return sb.st_size;
    }

size_t lines_count(const char *buf)
    {
    assert(buf != 0);

    size_t count = 0;

    for (; *buf != '\0'; buf++)
        {
        if (*buf == '\n')
            {
            count++;
            }
        }

    return count;
    }
