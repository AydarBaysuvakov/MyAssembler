#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "../headers/constants.h"
#include "stack.h"
#include "hash.h"

error_t MyStackCtor(Stack *stk, const char* name, const unsigned line, const char* file, const char* func)
    {
    assert(stk != NULL);

    stk->size     = DEFAULT_SIZE;
    stk->capacity = DEFAULT_CAPACITY;

    char *data = (char*) calloc(DEFAULT_CAPACITY * sizeof(Elem_t) + TWO_CANARY * sizeof(Canary_t), sizeof(char)); //calloc
    if (data == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    stk->data = (Elem_t*) (data + ONE_CANARY * sizeof(Canary_t));
    FillStack(stk);

    stk->name = name;
    stk->line = line;
    stk->file = file;
    stk->func = func;

    if (StkLogFileInit(stk, name) == FileError)
        {
        perror("ERROR: cannot open logfile");
        return FileError;
        }

#ifdef STACK_CANARY_PROT
    stk->left_canary  = LEFT_CANARY_VALUE;
    stk->right_canary = RIGHT_CANARY_VALUE;
#endif

#ifdef STACK_HASH_PROT
    SetHash(stk);
#endif

    return Ok;
    }

error_t StackDtor(Stack *stk)
    {
#ifdef DEBUG
    StackState state = StackValid(stk);
    if (state)
        {
        StackDump(stk, state);
        }
#endif

    free(GetDataLeftCanary(stk));

    stk->size     = NOT_VALID_VALUE;
    stk->capacity = NOT_VALID_VALUE;

#ifdef STACK_CANARY_PROT
    stk->left_canary  = NOT_VALID_VALUE;
    stk->right_canary = NOT_VALID_VALUE;
#endif

#ifdef STACK_HASH_PROT
    stk->data_hash  = NOT_VALID_VALUE;
    stk->stack_hash = NOT_VALID_VALUE;
#endif

    fclose(stk->logfile);

    return Ok;
    }

error_t StkLogFileInit(Stack *stk, const char* name)
    {
    char file_name[LOG_FILE_MAX_NAME_LENGHT] = "logfiles/logfile(";
    strncat(file_name,  name   , LOG_FILE_NAME_LENGHT);
    strncat(file_name, ").html", LOG_FILE_NAME_LENGHT);

    stk->logfile = fopen(file_name, "w");
    if (stk->logfile == NULL)
        {
        return FileError;
        }

    return Ok;
    }

error_t FillStack(Stack *stk)
    {
#ifdef DEBUG
    assert(stk != NULL);
    assert(stk->data != NULL);
    assert(stk->size <= stk->capacity);
#endif

#ifdef STACK_CANARY_PROT
    Canary_t *left_canary  = GetDataLeftCanary(stk);
    Canary_t *right_canary = GetDataRightCanary(stk);

    *left_canary  = DATA_LEFT_CANARY_VALUE;
    *right_canary = DATA_RIGHT_CANARY_VALUE;
#endif

    for (size_t iter = stk->size; iter < stk->capacity; iter++)
        {
        stk->data[iter] = POISON;
        }

    return Ok;
    }

error_t StackPush(Stack *stk, Elem_t value)
    {
#ifdef DEBUG
    StackState state = StackValid(stk);
    if (state)
        {
        StackDump(stk, state);
        }
#endif

    if (stk->size == stk->capacity)
        {
        StackRealloc(stk, stk->capacity * REALLOC_COEFFICIENT);
        }

    stk->data[stk->size++] = value;

#ifdef STACK_HASH_PROT
    SetHash(stk);
#endif

    return Ok;
    }

Elem_t StackPop(Stack *stk)
    {
#ifdef DEBUG
    StackState state = StackValid(stk);
    if (state)
        {
        StackDump(stk, state);
        }
#endif

    if (stk->size == 0)
        {
        printf("STACK is empty\n");
        exit(1);
        return POISON;
        }

    Elem_t ret_val = stk->data[--stk->size];
    stk->data[stk->size] = POISON;

    if (stk->size <= stk->capacity / REALLOC_DOWN_BORDER && stk->capacity > MIN_REALLOC_DOWN_SIZE)
        {
        StackRealloc(stk, stk->capacity / REALLOC_COEFFICIENT);
        }

#ifdef STACK_HASH_PROT
    SetHash(stk);
#endif

    return ret_val;
    }

Elem_t StackTop(const Stack *stk)
    {
#ifdef DEBUG
    StackState state = StackValid(stk);
    if (state)
        {
        StackDump(stk, state);
        }
#endif

    if (stk->size == 0)
        {
        printf("STACK is empty\n");
        return POISON;
        }

    return stk->data[stk->size - 1];
    }

error_t StackRealloc(Stack* stk, size_t new_capacity)
    {
#ifdef DEBUG
    StackState state = StackValid(stk);
    if (state)
        {
        StackDump(stk, state);
        }
#endif

    char *data = (char*) realloc(GetDataLeftCanary(stk), new_capacity * sizeof(Elem_t) + TWO_CANARY * sizeof(Canary_t));
    if (data == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    stk->data     = (Elem_t*) (data + ONE_CANARY * sizeof(Canary_t));
    stk->capacity = new_capacity;

    FillStack(stk);
    return Ok;
    }

StackState StackValid(Stack *stk)
    {
    StackState state = 0;

    if (stk == NULL)                                           // 1
        {
        state |= STK_NULLPTR;
        return state;
        }
    if (stk->data == NULL)                                     // 2
        {
        state |= STK_DATA_NULL;
        }
    if (stk->size > stk->capacity)                             // 3
        {
        state |= STK_INVALID_SIZE;
        }
    if (stk->size != 0 && stk->data[stk->size - 1] == POISON)  // 4
        {
        state |= STK_POISON_VAL;
        }
#ifdef STACK_CANARY_PROT
    if (stk->left_canary != LEFT_CANARY_VALUE)                 // 5
        {
        state |= LEFT_CANARY_CILLED;
        }
    if (stk->right_canary != RIGHT_CANARY_VALUE)               // 6
        {
        state |= RIGHT_CANARY_CILLED;
        }
    if (*GetDataLeftCanary(stk) != DATA_LEFT_CANARY_VALUE)      // 7
        {
        state |= DATA_LEFT_CANARY_CILLED;
        }
    if (*GetDataRightCanary(stk) != DATA_RIGHT_CANARY_VALUE)   // 8
        {
        state |= DATA_RIGHT_CANARY_CILLED;
        }
#endif
#ifdef STACK_HASH_PROT
    if (DataHashCheck(stk))                                    // 9
        {
        state |= DATA_HASH_NOT_COMPARE;
        }
    if (StackHashCheck(stk))                                   // 10
        {
        state |= STACK_HASH_NOT_COMPARE;
        }
#endif
    return state;
    }

error_t MyStackDump(const Stack *stk, StackState state, const char* name, const unsigned line, const char* file, const char* func)
    {
    fprintf(stk->logfile, "<pre>\n\n");

    fprintf(stk->logfile, "Stack[%p] '%s' from %s(%u) %s()\n", stk, stk->name, stk->file, stk->line, stk->func);
    fprintf(stk->logfile, "\tcalled like '%s' from %s(%u) %s()\n",       name,      file,      line,      func);
    fprintf(stk->logfile, "\t{\n\tsize = %lu\n\tcapacity = %lu\n\tdata[%p]\n\t\t{\n", stk->size, stk->capacity, stk->data);

    for (size_t iter = 0; iter < stk->capacity; iter++)
        {
        fprintf(stk->logfile, (stk->data[iter] != POISON) ? "\t\t*" : "\t\t ");
        fprintf(stk->logfile, "[%lu] = ", iter);
        fprintf(stk->logfile, (stk->data[iter] == POISON) ? "POISON\n" : "%d\n", stk->data[iter]);
        }
    fprintf(stk->logfile, "\t\t}\n");

#ifdef STACK_CANARY_PROT
    fprintf(stk->logfile, "\t\tData left canary  = %llX\n",   *GetDataLeftCanary(stk));
    fprintf(stk->logfile, "\t\tData right canary = %llX\n\n", *GetDataRightCanary(stk));

    fprintf(stk->logfile, "\tLeft canary  = %llX\n", stk->left_canary);
    fprintf(stk->logfile, "\tRight canary = %llX\n", stk->right_canary);
#endif

    fprintf(stk->logfile, "\t}\n\n");

    for(size_t bit = 0; bit < CHAR_BIT * sizeof(state); bit++)
        {
        if (state & 1 << bit)
            {
            fprintf(stk->logfile, "%s\n", GetStackErrorBitMsg(bit));
            }
        }

    fprintf(stk->logfile, "</pre>\n\n");

    return Ok;
    }

const char* GetStackErrorBitMsg(size_t bit)
{
    const int  ERROR_COUNT = sizeof(StackState) * CHAR_BIT;
    const char * const ERROR_MESSAGE[ERROR_COUNT] = {
        "Stack is nullptr",
        "Stack->data is nullptr",
        "Stack->size > Stack->capacity",
        "Curent value is poison",
        "Left canary killed",
        "Right canary killed",
        "Data left canary killed",
        "Data right canary killed",
        "Data hash not compare",
        "Stack hash not compare"
    };

    return ERROR_MESSAGE[bit];
}

#ifdef STACK_CANARY_PROT
Canary_t* GetDataLeftCanary(const Stack *stk)
    {
    return (Canary_t*) ((char*) stk->data - ONE_CANARY * sizeof(Canary_t));
    }

Canary_t* GetDataRightCanary(const Stack *stk)
    {
    return (Canary_t*) (stk->data + stk->capacity);
    }
#else
Elem_t* GetDataLeftCanary(const Stack *stk)
    {
    return stk->data;
    }

Elem_t* GetDataRightCanary(const Stack *stk)
    {
    return stk->data + stk->capacity;
    }
#endif

#ifdef STACK_HASH_PROT
Hash_t StackHashFunction(const Stack *stk)
    {
    assert(stk != NULL);

    return HashFunction((char*) stk, SIZE_OF_STACK_STRUCT);
    }

Hash_t DataHashFunction(const Stack *stk)
    {
    assert(stk != NULL);

    return HashFunction((char*) stk->data - ONE_CANARY, stk->capacity + TWO_CANARY);
    }

error_t SetHash(Stack *stk)
    {
    assert(stk != NULL);

    stk->data_hash  =  DataHashFunction(stk);

    stk->stack_hash = DEFAULT_HASH_VALUE;
    stk->stack_hash = StackHashFunction(stk);

    return Ok;
    }

error_t StackHashCheck(Stack *stk)
    {
    assert(stk != NULL);

    Hash_t curent_stk_hash = stk->stack_hash;
    stk->stack_hash = DEFAULT_HASH_VALUE;
    Hash_t stack_hash = StackHashFunction(stk);
    if (stack_hash != curent_stk_hash)
        {
        return HashNotCompare;
        }
    stk->stack_hash = curent_stk_hash;

    return Ok;
    }

error_t DataHashCheck(Stack *stk)
    {
    assert(stk != NULL);

    Hash_t data_hash = DataHashFunction(stk);
    if (data_hash != stk->data_hash)
        {
        return HashNotCompare;
        }

    return Ok;
    }


error_t HashCheck(Stack *stk)
    {
    assert(stk != NULL);

    if ( DataHashCheck(stk)) return HashNotCompare;
    if (StackHashCheck(stk)) return HashNotCompare;

    return Ok;
    }
#endif
