#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "errors.h"
#include "oneginlib.h"

Error_t TextCtor(Text *text, const char *file_name)
    {
    assert(text != NULL);

    MakeBuf(text, file_name);
    LinesPartition(text);

    return Ok;
    }

Error_t TextDtor(Text *text)
    {
    assert(text != NULL);

    free(text->buf);
    free(text->lines);

    text->buf_size = EMPTY;
    text->n_lines  = EMPTY;

    return Ok;
    }

Error_t MakeBuf(Text *text, const char *file_name)
    {
    assert(text      != NULL);
    assert(file_name != NULL);

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
        {
        perror("ERROR: cannot open file");
        return FileError;
        }

    text->buf_size = FileSize(fp);
    if (text->buf_size == -1)
        {
        perror("ERROR: fstat() func returned -1");
        return FileError;
        }

    text->buf = (char*) malloc((text->buf_size + 1) * sizeof(char));
    if (text->buf == nullptr)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    if (FillBuf(text->buf, text->buf_size, fp))
        {
        perror("ERROR: buffer overflow");
        return BufferOverflowError;
        }

    if (fclose(fp) == EOF)
        {
        perror("ERROR: cannot close file");
        return FileError;
        }

    return Ok;
    }

Error_t FillBuf(char *buf, size_t buf_size, FILE *fp)
    {
    assert(fp  != NULL);
    assert(buf != NULL);

    fread(buf, sizeof(*buf), buf_size, fp);

    return Ok;
    }

Error_t LinesPartition(Text *text)
    {
    assert(text      != NULL);
    assert(text->buf != NULL);

    text->n_lines = LinesCount(text->buf) + 1;

    text->lines = (char**) malloc((text->n_lines + 1) * sizeof(char*));
    if (text->lines == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    if (FillLines(text))
        {
        perror("ERROR: buffer overflow");
        return BufferOverflowError;
        }

    return Ok;
    }

Error_t FillLines(Text *text)
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
            return BufferOverflowError;
            }
        if (line - text->lines > text->n_lines)
            {
            return BufferOverflowError;
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

    return Ok;
    }

Error_t TextToFile(Text* text, FILE* fp)
    {
    assert(text != NULL);
    assert(fp   != NULL);

    char** line = text->lines;

    for (; *line != NULL; line++)
        {
        fprintf(fp, "%s \n", *line);
        }

    return Ok;
    }

Error_t PrintText(Text* text, const char* file_name)
    {
    assert(text        != NULL);
    assert(text->buf   != NULL);
    assert(text->lines != NULL);

    if (file_name == NULL)
        {
        TextToFile(text);
        return Ok;
        }

    FILE *fp = fopen(file_name, "w");
    TextToFile(text, fp);
    fclose(fp);

    return Ok;
    }

size_t FileSize(FILE *fp)
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

size_t LinesCount(const char *buf)
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
