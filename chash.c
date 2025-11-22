#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"
#include <pthread.h>
#include "hashrec.h"
#include "command.h"

char fileName[] = "commands.txt";

// tcb_t* createThread(command_t* cmd) {}

int main(int argc, char *argv[]) {
    printf("Concurrent Hash Table\n");

    int temp = 0;
    int *numCmd = &temp;
    command_t *head = buildCmdLL(fileName, numCmd);
    // printLL(head);
    // printf("numCmd: %d\n", *numCmd);

    wait_queue.threads = (tcb_t*)malloc(sizeof(tcb_t) * (*numCmd));

    command_t* current = head;
    while(current != NULL) {
        // tcb_t* newThread = createThread()
        // add_to_queue(tcb_args);
    }

    // ...

    free(wait_queue.threads);

    return 0;
}
