#pragma once
#include "hashrec.h"
#include <string.h>
#include <stdint.h>

const int TABLE_SIZE = 512;

typedef struct hash_table {
    hashRecord* head;
    size_t size;
} HashTable;

/*
Global hashtable pointer
*/
HashTable * htp;

// Initialize a new hashtable.
HashTable* initHashTable() {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = 0;
    table->head = NULL;
    return table;
}

// Free the hashtable and all its entries.
void freeHashTable(HashTable* table) {
    if(table == NULL) return;
    hashRecord* current = table->head;
    hashRecord* next;
    while(current != NULL) {
        next = current->next;
        freeHashRecord(current);
        current = next;
    }
    free(table);
}
