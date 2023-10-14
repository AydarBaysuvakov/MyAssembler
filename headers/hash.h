#ifndef HASH_H
#define HASH_H

typedef long long Hash_t;

const Hash_t DEFAULT_HASH_VALUE = 0xA1DA2BA15;

Hash_t  HashFunction(const char *array, size_t size);

#endif//HASH_H_
