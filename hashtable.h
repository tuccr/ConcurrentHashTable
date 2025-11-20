#include "hashrec.h"
#include <string.h>
#include <stdint.h>

const int TABLE_SIZE = 512;

typedef struct hash_table {
    hashRecord** records;
    size_t size;
} HashTable;

/*
Global hashtable pointer
*/
HashTable * htp;

HashTable* initHashTable(size_t size) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = size;
    table->records = (hashRecord**)calloc(size, sizeof(hashRecord*));
    return table;
}

void freeHashTable(HashTable* table) {
    for(size_t i = 0; i < table->size; i++) {
        hashRecord* current = table->records[i];
        while(current != NULL) {
            hashRecord* toFree = current;
            current = current->next;
            freeHashRecord(toFree);
        }
    }
    free(table->records);
    free(table);
}

uint32_t realHash(const uint32_t hash, size_t tableSize) {
    return hash % tableSize;
}

