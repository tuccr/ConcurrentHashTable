#include "hashtable.h"
#include "command.h"
#include "logging.h"
#include "threads.h"
#include <pthread.h>
#include <semaphore.h>


typedef struct _rwlock_t {
    sem_t writelock;
    sem_t lock;
    int readers;
} rwlock_t;

void rwlock_init(rwlock_t *lock) {
    lock->readers = 0;
    Sem_init(&lock->lock, 1); 
    Sem_init(&lock->writelock, 1); 
}

void rwlock_acquire_readlock(rwlock_t *lock) {
    Sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
	Sem_wait(&lock->writelock);
    Sem_post(&lock->lock);
}

void rwlock_release_readlock(rwlock_t *lock) {
    Sem_wait(&lock->lock);
    lock->readers--;
    if (lock->readers == 0)
	Sem_post(&lock->writelock);
    Sem_post(&lock->lock);
}

void rwlock_acquire_writelock(rwlock_t *lock) {
    Sem_wait(&lock->writelock);
}

void rwlock_release_writelock(rwlock_t *lock) {
    Sem_post(&lock->writelock);
}


wait_queue_t wait_queue;
rwlock_t rwlock;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
Waits until thread with current priority can proceed.
*/
void wait_turn(command_t* cmd) {
    log_event(WAIT, cmd->priority);
    pthread_mutex_lock(&mutex);
    while(get_highest_priority(&wait_queue) != cmd->priority) {
        pthread_cond_wait(&cv, &mutex);
    }
    log_event(AWAKENED, cmd->priority);
    log_cmd(cmd);
    pthread_mutex_unlock(&mutex);
}

/*
Accepts command_t struct pointer and inserts into hashtable.
*/
void* insert(void* arg) {
    command_t* cmd = (command_t*)arg;
    
    wait_turn(cmd);
    pthread_cond_signal(&cv); // may need to be broadcast?

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

    return NULL;

}


/*
Accepts command_t struct pointer and searches hashtable for entry. Returns pointer to hashRecord if found, NULL otherwise.
*/
void* search(void* arg) {
    command_t* cmd = (command_t*)arg;

    log_event(WAIT, cmd->priority);
    
    wait_turn(cmd);
    pthread_cond_signal(&cv); // may need to be broadcast?

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));

    rwlock_acquire_readlock(&rwlock);
    log_event(READ_LOCK_ACQUIRE, cmd->priority);

    hashRecord* current = htp->head;
    while(current != NULL) {
        if(current->hash == hash && strcmp(current->name, cmd->name) == 0) {
            rwlock_release_readlock(&rwlock);
            log_event(READ_LOCK_RELEASE, cmd->priority);
            return (void*)current;
        }
        current = current->next;
    }

    rwlock_release_readlock(&rwlock);

    return NULL;
}


/*
Accepts command_t struct pointer and updates salary in hashtable entry if found.
*/
void* updateSalary(void* arg) {
    command_t* cmd = (command_t*)arg;

    wait_turn(cmd);
    pthread_cond_signal(&cv); // may need to be broadcast?
    pthread_cond_signal(&cv);

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
        printf("Update failed: entry for %s not found\n", cmd->name);
    }

    rwlock_release_writelock(&rwlock);
    log_event(WRITE_LOCK_RELEASE, cmd->priority);

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