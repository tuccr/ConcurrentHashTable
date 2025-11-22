#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"
#include <pthread.h>
#include "hashrec.h"
#include "command.h"

char fileName[] = "commands.txt";

tcb_t* createThread(command_t* cmd) {
tcb_t* newThread = malloc(sizeof(tcb_t));
// newThread->priority = cmd->priority; prio removed
newThread->cond = malloc(sizeof(pthread_cond_t));
switch (cmd) {
    case INSERT:
        pthread_create(&newThread->thread_id, NULL, insert, cmd);
        break;
    case DELETE:
        pthread_create(&newThread->thread_id, NULL, delete, cmd);
        break;
    case UPDATE:
        pthread_create(&newThread->thread_id, NULL, update, cmd);
        break;
    case SEARCH:
        pthread_create(&newThread->thread_id, NULL, search, cmd);
        break;
    case PRINT:
        pthread_create(&newThread->thread_id, NULL, print, cmd);
        break;
    default:
        printf("Invalid command type for thread creation\n");
        break;
}
return newThread;
}

int main(int argc, char *argv[]) {
    printf("Concurrent Hash Table\n");

    int temp = 0;
    int *numCmd = &temp;
    command_t *head = buildCmdLL(fileName, numCmd);

    // printLL(head);
    printLL(head);
    // printf("numCmd: %d\n", *numCmd);
    printf("number of commands: %d\n", *numCmd);

    wait_queue.threads = (tcb_t*)malloc(sizeof(tcb_t) * (*numCmd));
    wait_queue.count = 0;

    pthread_t * tids = (pthread_t *)malloc(sizeof(pthread_t) * (*numCmd));

    command_t* current = head;
    int n = 0;
    while(current != NULL) {
        // tcb_t* newThread = createThread()
        tcb_t* newThread = createThread(current);
        tids[n] = newThread->thread_id;
        n++;
        // add_to_queue(tcb_args);
        add_to_queue(&wait_queue, newThread->thread_id, current->priority); 
        current = current->next;
    }

    for(int i = 0; i < n; i++) {
        pthread_join(tids[i], NULL);
    }

    // ...

    printf("Queue contains %d threads\n", wait_queue.count);

    free(wait_queue.threads);

    return 0;
}
