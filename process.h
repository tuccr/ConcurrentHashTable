#pragma once
#include "hashtable.h"
#include "command.h"
#include "logging.h"
#include "threads.h"
#include <pthread.h>
#include <semaphore.h>

// rwlock struct from OSTEP
typedef struct _rwlock_t {
    sem_t writelock;
    sem_t lock;
    int readers;
} rwlock_t;

// OSTEP CODE //
void rwlock_init(rwlock_t *lock) {
    lock->readers = 0;
    sem_init(&lock->lock, 1, 1); 
    sem_init(&lock->writelock, 1, 1); 
}

void rwlock_acquire_readlock(rwlock_t *lock) {
    sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
	sem_wait(&lock->writelock);
    sem_post(&lock->lock);
}

void rwlock_release_readlock(rwlock_t *lock) {
    sem_wait(&lock->lock);
    lock->readers--;
    if (lock->readers == 0)
	sem_post(&lock->writelock);
    sem_post(&lock->lock);
}

void rwlock_acquire_writelock(rwlock_t *lock) {
    sem_wait(&lock->writelock);
}

void rwlock_release_writelock(rwlock_t *lock) {
    sem_post(&lock->writelock);
}
// END OF OSTEP CODE

wait_queue_t wait_queue; // queue of waiting threads
rwlock_t rwlock; // rwlock struct
pthread_cond_t cv = PTHREAD_COND_INITIALIZER; // cv for priority checking and waking
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for atomic cv checking

/*
Waits until thread with current priority can proceed.
*/
void wait_turn(command_t* cmd) {
    log_event(WAIT, cmd->priority);
    pthread_mutex_lock(&mutex);
    while(get_highest_priority(&wait_queue) != cmd->priority) {
        pthread_cond_wait(&cv, &mutex);
    }
    remove_from_queue(&wait_queue, pthread_self());
    log_event(AWAKENED, cmd->priority);
    log_cmd(cmd);
    pthread_mutex_unlock(&mutex);
    pthread_cond_broadcast(&cv); 
}

/*
Accepts command_t struct pointer and inserts into hashtable.
*/
void* insert(void* arg) {
    command_t* cmd = (command_t*)arg;
    
    wait_turn(cmd);
    log_event(AWAKENED, cmd->priority);

    hashRecord * record = newHashRecord((uint8_t *)(cmd->name), (uint32_t)(cmd->salary));

    rwlock_acquire_writelock(&rwlock);
    log_event(WRITE_LOCK_ACQUIRE, cmd->priority);

    if(htp->head == NULL) {
        htp->head = record;
        htp->size += 1;

        rwlock_release_writelock(&rwlock);
        log_event(WRITE_LOCK_RELEASE, cmd->priority);
        return NULL;
    }

    hashRecord* current = htp->head;
    while(current->next != NULL) {
        if(current->hash == record->hash && strcmp(current->name, record->name) == 0) {
            freeHashRecord(record);
            
            printf("Insert failed: duplicate entry for %s\n", record->name);
            
            rwlock_release_writelock(&rwlock);
            log_event(WRITE_LOCK_RELEASE, cmd->priority);
            return NULL;
        }
        current = current->next;
    }
    current->next = record;
    htp->size += 1;

    rwlock_release_writelock(&rwlock);
    log_event(WRITE_LOCK_RELEASE, cmd->priority);

    char buffer[256];
    snprintf(buffer, 256, "Inserted %d,%s,%d\n", record->hash, record->name, record->salary);
    protected_print(buffer);

    return NULL;

}


/*
Accepts command_t struct pointer and searches hashtable for entry. Returns pointer to hashRecord if found, NULL otherwise.
*/
void* search(void* arg) {
    command_t* cmd = (command_t*)arg;
    char buffer[256];

    log_event(WAIT, cmd->priority);
    
    wait_turn(cmd);
    log_event(AWAKENED, cmd->priority);

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    rwlock_acquire_readlock(&rwlock);
    log_event(READ_LOCK_ACQUIRE, cmd->priority);

    hashRecord* current = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            snprintf(buffer, 256, "Found: %d,%s,%d\n", current->hash, current->name, current->salary);
            protected_print(buffer);
            rwlock_release_readlock(&rwlock);
            log_event(READ_LOCK_RELEASE, cmd->priority);
            return (void*)current;
        }
        current = current->next;
    }

    rwlock_release_readlock(&rwlock);
    log_event(READ_LOCK_RELEASE, cmd->priority);
    
    
    snprintf(buffer, 256, "%s not found.\n", cmd->name);
    protected_print(buffer);
    
    return NULL;
}


/*
Accepts command_t struct pointer and updates salary in hashtable entry if found.
*/
void* updateSalary(void* arg) {
    command_t* cmd = (command_t*)arg;

    wait_turn(cmd);
    log_event(AWAKENED, cmd->priority);

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    rwlock_acquire_writelock(&rwlock);
    log_event(WRITE_LOCK_ACQUIRE, cmd->priority);

    hashRecord* current = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            break;
        }
        current = current->next;
    }

    if(current != NULL) {
        current->salary = (uint32_t)(cmd->salary);
    }
    else {
        printf("Update failed. Entry %s not found\n", cmd->name);
    }

    rwlock_release_writelock(&rwlock);
    log_event(WRITE_LOCK_RELEASE, cmd->priority);
    
    char buffer[256];
    snprintf(buffer, 256, "Updated record %d from %d,%s,%d to %d,%s,%d\n", current->hash, current->hash, current->name, current->salary, current->hash, current->name, current->salary);
    protected_print(buffer);

    return NULL;
}


/*
Accepts command_t struct pointer and deletes entry from hashtable if found.
*/
void* delete(void* arg) {
    command_t* cmd = (command_t*)arg;

    log_event(WAIT, cmd->priority);

    wait_turn(cmd);
    log_event(AWAKENED, cmd->priority);

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    // can't just use search for this one since we need to modify previous pointers

    rwlock_acquire_readlock(&rwlock);
    
    hashRecord* current = htp->head;
    hashRecord* prev = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            rwlock_acquire_writelock(&rwlock);
            log_event(WRITE_LOCK_ACQUIRE, cmd->priority);
            prev->next = current->next;
            freeHashRecord(current);
            htp->size -= 1;
            rwlock_release_writelock(&rwlock);
            log_event(WRITE_LOCK_RELEASE, cmd->priority);
            char buffer[256];
            snprintf(buffer, 256, "Deleted record for %d,%s,%d\n", hash, cmd->name, cmd->salary);
            protected_print(buffer);
            return NULL;
        }
        prev = current;
        current = current->next;
    }
    
    char buffer[256];
    snprintf(buffer, 256, "Entry %d not deleted. Not in database\n", hash);
    protected_print(buffer);

    rwlock_release_readlock(&rwlock);
    log_event(READ_LOCK_RELEASE, cmd->priority);

    return NULL;
}

void* print(void* arg) {
    command_t* cmd = (command_t*)arg;
    
    log_event(WAIT, cmd->priority);
    wait_turn(cmd);

    log_event(AWAKENED, cmd->priority);

    rwlock_acquire_readlock(&rwlock);
    log_event(READ_LOCK_ACQUIRE, cmd->priority);

    printf("Current Database:\n");
    hashRecord* current = htp->head;
    while(current != NULL) {
        printf("%d,%s,%d\n", current->hash, current->name, current->salary);
        current = current->next;
    }

    rwlock_release_readlock(&rwlock);
    log_event(READ_LOCK_RELEASE, cmd->priority);

    char buffer[64];
    // print to stdout here

    return NULL;
}
