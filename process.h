#include "hashtable.h"
#include "command.h"
#include "logging.h"
#include "threads.h"

pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

wait_queue_t wait_queue;

/*
Accepts command_t struct pointer and inserts into hashtable.
*/
void* insert(void* arg) {
    command_t* cmd = (command_t*)arg;
    
    log_event(WAIT, cmd->priority);

    hashRecord * record = newHashRecord((uint8_t *)(cmd->name), (uint32_t)(cmd->salary));

    pthread_mutex_lock(&write_mutex);
    while(get_highest_priority(&wait_queue) != cmd->priority) {
        pthread_cond_wait(&cv, &write_mutex);
    }
    log_event(AWAKENED, cmd->priority);
    log_cmd(cmd);
    log_event(WRITE_LOCK_ACQUIRE, cmd->priority);


    if(htp->head == NULL) {
        htp->head = record;
    }

    hashRecord* current = htp->head;
    while(current->next != NULL) {
        if(current->hash == record->hash && strcmp(current->name, record->name) == 0) {
            freeHashRecord(record);
            
            printf("Insert failed: duplicate entry for %s\n", record->name);
            
            pthread_mutex_unlock(&write_mutex);
            log_event(WRITE_LOCK_RELEASE, cmd->priority);
            return NULL;
        }
        current = current->next;
    }
    current->next = record;
    htp->size += 1;

    // write_unlock()
    pthread_mutex_unlock(&write_mutex);
    log_event(WRITE_LOCK_RELEASE, cmd->priority);

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
    command_t* cmd = (command_t*)arg;

    pthread_mutex_lock(&read_mutex);
    while(get_highest_priority(&wait_queue) != cmd->priority) {
        pthread_cond_wait(&cv, &read_mutex);
    }
    log_event(AWAKENED, cmd->priority);
    log_cmd(cmd);
    log_event(READ_LOCK_ACQUIRE, cmd->priority);
    pthread_cond_signal(&cv);

    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t *)(cmd->name), strlen(cmd->name));
    // read_lock()
    log_event(READ_LOCK_ACQUIRE, cmd->priority);
    hashRecord* toChange = (hashRecord*)search(arg);
    // read_unlock()
    log_event(READ_LOCK_RELEASE, cmd->priority);

    // write_lock()
    log_event(WRITE_LOCK_ACQUIRE, cmd->priority);
    if(toChange != NULL) {
        toChange->salary = (uint32_t)(cmd->salary);
    }

    // write_unlock()
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