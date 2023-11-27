#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "errors.h"
#include "logfiles.h"

Error_t LogFileInit(FILE **file, const char* prefix, const char* name, const char* extension)
    {
    assert(name != NULL);
    assert(prefix != NULL);
    assert(extension != NULL);

    char file_name[LOG_FILE_NAME_LENGTH] = "logfiles/";
    strncat(file_name,  prefix  , LOG_FILE_NAME_LENGTH);
    strncat(file_name,  "_"     , sizeof("_"));
    strncat(file_name,  name + 1, LOG_FILE_OBJ_NAME_LENGTH);
    strncat(file_name,  "."     , sizeof("."));
    strncat(file_name, extension, sizeof(extension));

    *file = fopen(file_name, "w");
    if (file == NULL)
        {
        return FileError;
        }

    return Ok;
    }
