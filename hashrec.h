#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "jenkins.h"

/*
Hash table entry struct.
*/
typedef struct hash_struct
{
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;


/*
Free existing hashRecord.
*/
void freeHashRecord(hashRecord* rec) {
    free(rec);
}

/*
Allocate memory for new hashRecord and return pointer to it. Calls jenkins_one_at_a_time_hash.
*/
hashRecord* newHashRecord(const uint8_t* name, const uint32_t salary) {
    hashRecord* newRec = (hashRecord*)malloc(sizeof(hashRecord));
    newRec->salary = salary;
    int len = 0;
    for(len; len < 50; len++) {
		newRec->name[len] = name[len]; // may need to typecast to char
		if(name[len] == '\0') break;
    }
    newRec->hash = jenkins_one_at_a_time_hash(name, len);
    newRec->next = NULL;

    return newRec;
}