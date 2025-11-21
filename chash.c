#include <stdio.h>
#include <string.h>
#include "hashrec.h"
#include "command.h"

char fileName[] = "commands.txt";


int main(int argc, char *argv[]) {
    printf("Concurrent Hash Table\n");

    int temp = 0;
    int *numCmd = &temp;
    command_t *head = buildCmdLL(fileName, numCmd);
    printLL(head);
    printf("numCmd: %d\n", *numCmd);







    return 0;
}
