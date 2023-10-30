#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "hash.h"

Hash_t HashFunction(const char *array, size_t size)
    {
    assert(array != NULL);

    Hash_t hash = 0;

    for (size_t byte = 0; byte < size; byte++)
        {
        for (size_t bit = 0; bit < CHAR_BIT; bit++)
            {
            hash += array[byte] & 1 << bit;
            }
        }

    return hash * hash / 2;
    }

