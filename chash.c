#include <stdio.h>
#include <string.h>
#include "hashrec.h"
#include "command.h"

char fileName[] = "commands.txt";


int main(int argc, char *argv[]) {
    printf("Concurrent Hash Table\n");

    command_t *head = buildCmdLL(fileName);
    printLL(head);

    return 0;
}
