#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "hash.h"

Hash_t HashFunction(const char *array, const int size)
    {
    assert(array != NULL);

    Hash_t hash = 0;

    for (size_t i = 0; i < size; i++)
    {
        hash += (unsigned char)(array[i]);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
    }

