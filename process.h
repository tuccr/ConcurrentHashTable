#include "hashtable.h"
#include "command.h"
#include "logging.h"
#include <pthread.h>

/*
Accepts command_t struct pointer and inserts into hashtable.
*/
void* insert(void* arg) {
    command_t* cmd = (command_t*)arg;

    hashRecord * newRec = newHashRecord((uint8_t *)(cmd->name), (uint32_t)(cmd->salary));

    uint32_t idx = realHash(newRec->hash, TABLE_SIZE);

    /*
    begin critical section
    */

    if(htp->records[idx] != NULL) {
        hashRecord * current = htp->records[idx];
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newRec;
    }
    else htp->records[idx] = newRec;

    /*
    End critical section
    */
    return NULL;
}


/*
Accepts command_t struct pointer and searches hashtable for entry. Returns pointer to hashRecord if found, NULL otherwise.
*/
void* search(void* arg) {
    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));
    uint32_t idx = realHash(hash, TABLE_SIZE);

    
    hashRecord* current = htp->records[idx];

    /*
    critical section start
    */

    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            // if we find it, we need to end critical section here
            return current;
        }
        current = current->next;
    }

    // also end critical section here if we don't find anything
    return NULL;
}


/*
Accepts command_t struct pointer and updates salary in hashtable entry if found.
*/
void* update(void* arg) {
    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));
    uint32_t idx = realHash(hash, TABLE_SIZE);

    /*
    Start critical section
    */

    hashRecord* toChange = (hashRecord*)search(arg);
    if(toChange != NULL) {
        toChange->salary = (uint32_t)(cmd->salary);
    }

    /*
    End critical section
    */

    return NULL;
}


/*
Accepts command_t struct pointer and deletes entry from hashtable if found.
*/
void* delete(void* arg) {
    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));
    uint32_t idx = realHash(hash, TABLE_SIZE);

    // can't just use search for this one since we need to modify previous pointers

    /*
    Begin critical section
    */
    hashRecord* current = htp->records[idx];
    hashRecord* prev = NULL;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            prev->next = current->next;
            freeHashRecord(current);
            return NULL;
        }
        prev = current;
        current = current->next;
    }

    /*
    End critical section
    */

    return NULL;
}