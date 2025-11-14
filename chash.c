#include <stdio.h>
#include <string.h>
#include "hashrec.h"

const char fileName[] = "commands.txt";

//Function Prototypes
int praseCmd();


int main(int argc, char *argv[]) {
    printf("Concurrent Hash Table\n");

    praseCmd();

    return 0;
}

// functions for search delete and all that





// supports 128 arguments
// Suppoerts up to 1024 character lines


// TODO: add error handling lol
int praseCmd()
{
    // uses the args from *argv[]
    FILE *file = fopen(fileName, "r");

    char *curLine[128];

    char temp[1024];
    while((fgets(temp, 1024, file)) != NULL) // while line in file is not EOF
    {
        // initialize the strtok on the current string
        char* curTok;
        curTok = strtok(temp, ",");
        printf("%s ", curTok);

        // assign first token to the buffer
        curLine[0] = curTok;

        int idx = 1;
        while( (curTok = strtok(NULL, ",")) != NULL)
        {
            curLine[idx] = curTok;
            printf("%s ", curTok);
        }
        printf("\n");

    }

    return 1;
}

void printSrtingArr(char **input)
{
}