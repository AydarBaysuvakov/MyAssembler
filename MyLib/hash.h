#ifndef HASH_H
#define HASH_H

typedef unsigned long Hash_t;

const Hash_t DEFAULT_HASH_VALUE = 0xA1DA2BA15;

Hash_t  HashFunction(const char *array, const int size);

#endif //HASH_H
