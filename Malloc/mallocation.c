#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>

/*
This program brought to you by: Aaron Mechanic
CS 340 Programming project ?
Memory allocation simulation

**Commented out code lines are likely debug. Please ignore them.
*/

#define MAX_BYTES 1048576
#define MAX_LINE_LENGTH 1024 /* The maximum length command */
#define MAX_TOKENS 256
static jmp_buf sh_start; //Global jump buffer

//Load order for methods matters. Declare all methods before main.

// Debuggers are for babies. Real men use distributed print statements.
void printdbg(char *str)
{
    // Simple printf statement and fflush so it always outputs even if the program crashes
    static int num = 0;
    fprintf(stderr, "%d: %s\n", num++, str);
    fflush(stdout);
}

struct Node // using a linked list
{
    int size;
    int start;
    int end;
    char *p;

    struct Node *next;
};

void init(struct Node **head_ref, int sizeB, char *pname, int startB)
{
    printdbg("Making a node in init");
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node)); // allocating memory for the nodes
    printdbg("After allocating memory");

    // Setters
    new_node->size = sizeB;
    printdbg("After setting size");
    new_node->start = startB;
    printdbg("After setting start");
    new_node->end = startB + sizeB;
    printdbg("After setting end");
    new_node->p = pname;
    printdbg("After setting p");
    new_node->next = (*head_ref); // gets here and breaks
    printdbg("After setting next");

    *head_ref = new_node;
    printdbg("After setting head_ref");
}

void insert(struct Node *prev_node, struct Node **next_node, int sizeB, char *pname, int startB)
{
    printdbg("Making a node in insert");
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));

    // Setters
    new_node->size = sizeB;
    new_node->start = startB;
    new_node->end = startB + sizeB;
    new_node->p = pname;
    new_node->next = *next_node;

    prev_node->next = new_node;
}

void release(struct Node *curr_node, struct Node *prev_node)
{
    prev_node->next = curr_node->next; // skip this node
    // by skipping the node we free up the space
    curr_node->next = NULL; // this just separates the node from the list
}

int main(void)
{
    int should_run = 1;
    char **args; // for args once parsed.

    struct Node *head_node = NULL; // head node
    int startpoints[10];
    int used = 0;

    while (should_run)
    {
        // setting a return point
        if (setjmp(sh_start) != 0)
        {
            printf("\n");
        }

        printf("mallocator> "); // Prompt string
        fflush(stdout);

        char *raw; // raw command
        size_t i = 0;

        raw = malloc(MAX_LINE_LENGTH); // allocating memory for the command
        args = malloc(MAX_TOKENS);
        // fgets(raw, MAX_LINE_LENGTH, stdin); // getting the user input
        read(STDIN_FILENO, raw, MAX_LINE_LENGTH); // taking in command
        int len = strlen(raw);                    // getting the length of the command

        if (len > 0 && raw[len - 1] == '\n') // dropping the newline at the end of the command
        {
            raw[len - 1] = '\0';
            len -= 1;
        }

        if (len == 1) // only C and X will be 1
        {
            if (strcmp(raw, "C") == 0)
            {
                printdbg("C entered");
            }
            else if (strcmp(raw, "X") == 0) // exit condition
            {
                should_run = 0;
            }
        }
        else
        {
            // tokenizing the args
            char *token = strtok(raw, " ");

            while (token != NULL)
            {
                printf("Token: %s \n", token);
                args[i++] = token;
                token = strtok(NULL, " ");
            }

            if (strcmp(args[0], "STAT") == 0) //looking for STAT
            {
                printdbg("STAT entered");

                struct Node *curr_node = head_node; // temp
                while (curr_node->next != NULL)     // looping through nodes
                {
                    printf("PID: %S Start: %d End: %d", curr_node->p, curr_node->start, curr_node->end);
                    curr_node = curr_node->next; // move to next node
                }

                longjmp(sh_start, EXIT_FAILURE); // Back to start
            }
            else if (strcmp(args[0], "RQ") == 0) // Looking for RQ
            {
                printdbg("RQ entered");

                printf("Input size: %d \n", atoi(args[2]));
                int inputSize = atoi(args[2]); // making life easier by parsing the number out of the string now

                if (used + inputSize > MAX_BYTES) // Asking for too much
                {
                    printf("Current used %d, asking for %d", used, inputSize);
                    printf("Insufficient memory");
                    longjmp(sh_start, EXIT_FAILURE); // back to start
                }
                else if (used == 0) // do we have any active memory use
                {
                    init(head_node, inputSize, args[1], used);
                    used = used + inputSize;         // increasing total used bytes
                    longjmp(sh_start, EXIT_FAILURE); // back to start
                }

                int start = 0;
                struct Node *current_node = head_node;
                struct Node *next_node = current_node->next; // nodes go in decreasing order. Next node has lower starting byte
                while (next_node != NULL)                    // this should not be null on first pass
                {
                    if (next_node->end + inputSize + 1 < current_node->start)
                    {
                        insert(next_node, current_node, inputSize, args[1], next_node->end);
                        used = used + inputSize;         // increasing total used bytes
                        longjmp(sh_start, EXIT_FAILURE); // back to start
                    }
                }
            }
            else if (strcmp(args[0], "RL") == 0) // looking for RL
            {
                printdbg("RL entered");
                struct Node *curr_node = head_node;
                struct Node *prev_node;

                while (curr_node->p != args[1] && curr_node->next == NULL) // while the process names dont match and another node exists move to next node
                {
                    prev_node = curr_node;
                    curr_node = curr_node->next;
                }
                if (curr_node->p == args[1]) // Double checking that the PID exists and we didnt go through all the nodes
                {
                    used = used - curr_node->size; // reducing the total used bytes
                    release(curr_node, prev_node);
                }
                else
                {
                    printf("No node by that process id"); // Couldn't find that PID
                    longjmp(sh_start, EXIT_FAILURE);      // back to start
                }
            }
        }
    }
    return 0;
}