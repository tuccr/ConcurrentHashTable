#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ARGLEN 1024
#define ARGNUM 128
#define LINELEN 1024 * 8 // 8kb

enum cmdType {INVALID = 0, INSERT = 1, DELETE = 2, UPDATE = 3, SEARCH = 4, PRINT = 5};

typedef struct command
{
    enum cmdType cmd;
	char name[128];
	int salary;
	int priority; // supporting negative priority and salary bc idk man

	struct command *next;
} command_t;


// function prototypes
command_t* initCommand(char *input);
command_t* buildCmdLL(char* fileName);  // creates a linked list from a command file
void printCommand(command_t *input);
int enumParse(char *input);
int printLL(command_t *head);
//int cmdDequeue(command_t *head);        // dont use this on its own lol
//int freeLL(command_t *head);            // frees entire list




// returns a command_t struct based on a line of input parsed from the input file
// the input string is [LINELEN] buffer Size
// returns NULL on error
command_t* initCommand(char *input)
{
    // error handling for NULL input string
    if(input == NULL)
    {
        return NULL;
    }

    // initialize to zero
    command_t *temp = (command_t*) malloc(sizeof(command_t));

    temp->cmd = INVALID;
    strcpy(temp->name, "");
    temp->salary = 0;        // stores new value in update
    temp->priority = 0;  
    temp->next = NULL;


    char* curTok;
    char curLine[ARGNUM][ARGLEN];



    // initialize the strtok on the current string
    curTok = strtok(input, ",\n");

    // error handling for empty input string
    if(curTok == NULL)
    {
        return NULL;
    }

    // assign first token to the buffer    
    strcpy(curLine[0], curTok);

    // write each word in the line to curLine
    int idx = 1;
    while( (curTok = strtok(NULL, ",\n")) != NULL)
    {
        strcpy(curLine[idx], curTok);
        //printf("\"%s\"  ", curTok);
        idx++;
    }



    // for debugging
    // print curLine
    printf("\ntest:\n");
    for(int i = 0; strcmp(curLine[i], ""); i++)
    {
        printf("\"%s\" ", curLine[i]);
    }
    printf("\n");




    // {INVALID = 0, INSERT = 1, DELETE = 2, UPDATE = 3, SEARCH = 4, PRINT = 5}
    switch (enumParse(curLine[0]))
    {
        case 1:
            temp->cmd = INSERT;
            strcpy(temp->name, curLine[1]);
            temp->salary = atoi(curLine[2]);
            temp->priority = atoi(curLine[3]);
            break;

        case 2:
            temp->cmd = DELETE;
            strcpy(temp->name, curLine[1]);
            temp->priority = atoi(curLine[2]);
            break;

        case 3:
            temp->cmd = UPDATE;
            strcpy(temp->name, curLine[1]);
            temp->salary = atoi(curLine[2]);  // store the new value in the struct
            break;

        case 4:
            temp->cmd = SEARCH;
            strcpy(temp->name, curLine[1]);
            temp->priority = atoi(curLine[2]);
            break;

        case 5:
            temp->cmd = PRINT;
            temp->priority = atoi(curLine[1]);
            break;

        default:
            temp->cmd = INVALID;
            printf("error assigning cmdType");
            break;

    }

    printCommand(temp);
    return temp;   // return as a pointer
}




// TODO: add error handling
// returns head* of a linked list of all commands in the file
command_t* buildCmdLL(char* fileName)
{

    char getLine[LINELEN];
    command_t *head = NULL;
    command_t *result = NULL;
    FILE *file = fopen(fileName, "r");

    if(file == NULL)
    {
        printf("Error opening file: \"%s\"\n", fileName);
        return NULL;
    }

    // initial Node
    if (fgets(getLine, LINELEN, file) == NULL)
    {
        return NULL;
    }
    else
    {
        result = initCommand(getLine);
        head = result; // idk maybe i need to do this probably
    }


    // fill in the rest of the linked list
    while((fgets(getLine, LINELEN, file)) != NULL) // while line in file is not EOF
    {
        
        command_t *temp = initCommand(getLine);

        // allocate the next struct in the line?
        head->next = temp;

        // advance to the next place entry in the linked list
        head = head->next;
    }

    return result;
}





void printCommand(command_t *input)
{
    char *cmdTypes[] = {"INVALID", "INSERT", "DELETE", "UPDATE", "SEARCH", "PRINT"};
    printf("Struct:\t\t%x\n", input);
    printf("Command:\t%s\n", cmdTypes[(input->cmd)]);
    printf("Name:\t\t%s\n", input->name);
    printf("Salary:\t\t%d\n", input->salary);
    printf("Priority:\t%d\n", input->priority);
    printf("Next:\t\t%x\n", input->next);
    printf("\n");
}


int enumParse(char *input)
{
    // enum cmdType {INVALID = 0, INSERT = 1, DELETE = 2, UPDATE = 3, SEARCH = 4, PRINT = 5};

    if(strcmp(input, "insert") == 0)
    {
        return 1;
    }


    if(strcmp(input, "delete") == 0)
    {
        return 2;
    }


    if(strcmp(input, "update") == 0)
    {
        return 3;
    }

    if(strcmp(input, "search") == 0)
    {
        return 4;
    }


    if(strcmp(input, "print") == 0)
    {
        return 5;
    }
    



    return 0;
}

int printLL(command_t *head)
{
    if(head == NULL)
    {
        printf("\ncommand_t = NULL\n");
        return 1;
    }

    printf("\n\nPrinting Linked List of Commands:\n\n");
    // base case
    if(head->next == NULL)
    {
        printCommand(head);
    }

    while(head->next != NULL)
    {
        printCommand(head);
        head = head->next;
    }
    return 0;
}

// int cmdDequeue(command_t *head)
// {   
//     if(head == NULL)
//     {
//         return 1;
//     }
//     else if(head->next == NULL) 
//     {
//         free(head);
//         return 0;
//     }

//     command_t *temp = head;
//     head = head->next;
//     free(temp);
//     return 0;
// }

// // frees the linked list recursively
// int freeLL(command_t *head)
// {
//     while(cmdDequeue(head) != 0)
//     {
//         continue;
//     }
// }







