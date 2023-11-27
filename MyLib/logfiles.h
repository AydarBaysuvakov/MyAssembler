#ifndef LOGFILE_H
#define LOGFILE_H

const int     LOG_FILE_NAME_LENGTH     = 32;
const int     LOG_FILE_OBJ_NAME_LENGTH = 8;

Error_t LogFileInit(FILE **file, const char* prefix, const char* name, const char* extension);

#endif //LOGFILE_H
