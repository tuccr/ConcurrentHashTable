#include "hashtable.h"
#include "command.h"
#include "logging.h"
#include <pthread.h>

/*
Accepts command_t struct pointer and inserts into hashtable.
*/
void* insert(void* arg) {
    // cv.wait()

    // cv.signal()

    command_t* cmd = (command_t*)arg;

    hashRecord * record = newHashRecord((uint8_t *)(cmd->name), (uint32_t)(cmd->salary));

    // write_lock()

    if(htp->head == NULL) {
        htp->head = record;
    }

    hashRecord* current = htp->head;
    while(current->next != NULL) {
        current = current->next;
    }
    current->next = record;
    htp->size += 1;

    // write_unlock()

    return NULL;

}


/*
Accepts command_t struct pointer and searches hashtable for entry. Returns pointer to hashRecord if found, NULL otherwise.
*/
void* search(void* arg) {
    // cv.wait()

    // cv.signal()
    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));
    uint32_t idx = realHash(hash, TABLE_SIZE);

    // read_lock()
    hashRecord* current = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            // read_unlock()
            return (void*)current;
        }
        current = current->next;
    }
    // read_unlock()

    return NULL;
}


/*
Accepts command_t struct pointer and updates salary in hashtable entry if found.
*/
void* update(void* arg) {
    // cv.wait()

    // cv.signal()
    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    // read_lock()
    hashRecord* toChange = (hashRecord*)search(arg);
    // read_unlock()

    // write_lock()
    if(toChange != NULL) {
        toChange->salary = (uint32_t)(cmd->salary);
    }

    // write_unlock()

    return NULL;
}


/*
Accepts command_t struct pointer and deletes entry from hashtable if found.
*/
void* delete(void* arg) {
    // cv.wait()

    // cv.signal()

    command_t* cmd = (command_t*)arg;

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    // can't just use search for this one since we need to modify previous pointers

    // read_lock()
    hashRecord* current = htp->head;
    hashRecord* prev = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            // read_unlock()
            // write_lock()
            prev->next = current->next;
            freeHashRecord(current);
            htp->size -= 1;
            // write_unlock()
            return NULL;
        }
        prev = current;
        current = current->next;
    }

    // read_unlock()

    return NULL;
}

void* print(void* args) {
    // cv.wait()

    // cv.signal()

    // read_lock()
    printf("Current Database:\n");
    hashRecord* current = htp->head;
    while(current != NULL) {
        printf("%d,%s,%d\n", current->hash, current->name, current->salary);
    }
    // read_unlock()

    return NULL;
}