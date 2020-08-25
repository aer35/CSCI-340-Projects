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
This linux shell brought to you by: Aaron Mechanic
CS 340 Programming project ?

**Commented out code lines are likely debug. Please ignore them.
*/

#define MAX_TOKENS 256
#define MAX_LINE_LENGTH 1024 /* The maximum length command */
static jmp_buf sh_start;     //Global jump buffer

// Load order matters in C
// Function must be delcared before Main or else compiler throws errors

// Debuggers are for babies. Real men use distributed print statements.
void printdbg(char *str)
{
    // Simple printf statement and fflush so it always outputs even if the program crashes
    static int num = 0;
    fprintf(stderr, "%d: %s\n", num++, str);
    fflush(stdout);
}

//This is the function for redirecting input and output with < or >
size_t ioRedirect(char **args, size_t argc)
{
    size_t argcOffset = 0;
    size_t lastArgcOffset = 0;

    for (size_t i = 0; i < argc; ++i)
    {
        // the character we're checking in the current string
        size_t charNum = 0;
        // the mode to open the new stream in
        char *mode = NULL;
        // the stream to redirect to a different file/stream
        FILE *streamToRedirect;

        // Check to see if we're redirecting input or output
        if (args[i][charNum] == '>')
        {
            streamToRedirect = stdout;
            mode = "w";
            ++charNum;
        }

        else if (args[i][charNum] == '<')
        { // redirect input
            streamToRedirect = stdin;
            mode = "r";
            ++charNum;
        }

        // whether we should remove 2 arguments instead of 1 (if there's a filename) after the I/O redirect.
        bool skipNextArg = false;

        // if the mode was set, a redirect was found
        if (mode != NULL)
        {
            // the file to redirect to, if used
            FILE *file = NULL;

            // the file descriptor to redirect to
            int fd;

            // add 1 to the offset for shifting
            ++argcOffset;

            // recall that i was decalred in the main loop of this function
            if (i + 1 >= argc)
            { // if not redirect argument exists throw an error
                fprintf(stderr, "smash: error: must specify file to redirect to");
                longjmp(sh_start, EXIT_FAILURE); // returns to a restore point
            }

            // open the file
            file = fopen(args[i + 1], mode);

            // if the file is NULL throw an error
            if (file == NULL)
            {
                fprintf(stderr, "smash: error: can't open file %s: ", args[i + 1]);
                perror("");                      // print from errno
                longjmp(sh_start, EXIT_FAILURE); // returns to a restore point
            }

            // get the file descriptor
            fd = fileno(file);

            // increment the args an extra time for the filename
            ++argcOffset;
            skipNextArg = true;

            // redirect the stream

            // if the dup2 returns a failure throw an error
            if (dup2(fd, fileno(streamToRedirect)) == -1)
            {
                fprintf(stderr, "smash: error: can't redirect %d to %d", fd, fileno(streamToRedirect));
                perror("");                      // print from errno
                longjmp(sh_start, EXIT_FAILURE); // returns to a restore point
            }

            // close the opened file, if needed
            if (file != NULL)
            {
                fclose(file);
            }

            // shift the array, skipping the redirect args
            size_t offset = argcOffset - lastArgcOffset;

            // if there was a filename, skip it too
            if (skipNextArg)
            {
                ++i;
            }
        }
        else
        {
            // just shift the array
            args[i - argcOffset] = args[i];
        }

        lastArgcOffset = argcOffset;
    }
    // null terminate the array
    args[argc - argcOffset] = NULL;
    return argc - argcOffset;
}

size_t hasPipe(char **args)
{
    size_t k = 0;
    while (strcmp(args[k], "|"))
    {
        // no pipe exists, end
        if (args[++k] == NULL)
        {
            // printdbg("no pipe");
            return -1;
        }
    }
    // replace the pipe with a NULL so it's the end of the array
    args[k] = NULL;
    return (k + 1);
}

void executecommand(char **args, int amp, size_t argc)
{
    // variable declarations
    size_t offset = hasPipe(args);
    char **args2;
    int fds[2];
    bool madePipe = false;

    // Offset is set by the madepipe method.
    // The method is set to return -1 if there is no pipe and the location of the pipe otherwise
    if (offset != -1)
    {
        /* Removed Debug statements
        // printf("%ld\n", offset);
        // printdbg("offset exits");
        // int n = 0;
        // while (args[offset] != NULL)
        // {
        //     printf(args[offset]);
        //     printf("\nText going into args2 ^^");
        //     args2[n] = args[offset];
        //     n++;
        //     offset++;
        // } */
        args2 = (args + offset);
        // printdbg("Do we get to here?");
        pipe(fds);
        madePipe = true;
    }

    // Creating the first child process
    pid_t bigPID = fork();

    // If we're in the child
    if (bigPID == 0)
    {
        // If we have a pipe
        if (madePipe)
        {
            // printdbg("in child made pipe");
            // Close the read end of the file descriptor
            close(fds[0]);
            // aliasing std_fileno to file descriptor 1 which is the write end
            dup2(fds[1], STDOUT_FILENO);
        }
        // printdbg("Make it out of child made pipe");
        else
        {
            // If we dont have a pipe execute the ioredirect method.
            // this wont do anything if there is no I/O redirect
            ioRedirect(args, argc);
        }
        int n = 0;
        /* Removed debug statements
        // while (args[n] != NULL)
        // {
        //     printf(args[n]);
        //     printf("\nText going into args ^^");
        //     n++;
        // } 
        // printdbg("before execvp"); */

        // When we're dont checking for I/O redirects or messing with the pipes execute the command and exit the child
        execvp(args[0], args);
        // printdbg("After exec 1");
        fflush(stdout);
        exit(0);
    }

    // If we are not in the child and dont have an apersand, wait. If we have a pipe, also close the write end of the file descriptor
    else if (!amp)
    {

        int q;
        wait(&q);
        if (madePipe)
        {
            close(fds[1]);
        }
    }

    // If none of the above just output the Process ID
    else
    {
        printf("%d \n", bigPID);
    }

    // If we had a pipe then we need to execute the second command by creating another child process.
    if (madePipe)
    {
        bigPID = fork();

        if (bigPID == 0)
        {
            // printdbg("in second child");
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            // printdbg("dup2 2 success");

            int m = 0;
            // while (args[m] != NULL)
            // {
            //     printf(args2[m]);
            //     printf(" %d\n", m);
            //     fflush(stdout);
            //     m++;
            // }

            // char *str;
            // while (read(fds[0], str, 100) != 0)
            // {
            //     printf(str);
            //     fflush(stdout);
            // }

            execvp(args2[0], args2);
            // printdbg("second command execute");
            fflush(stdout);
            exit(0);
        }

        // If we're no longer in the second child then wait
        else
        {
            int q;
            wait(&q);
            close(fds[0]);
        }
    }
}

// Main
int main(void)
{
    char **args;        // command line arguments
    int should_run = 1; // flag to determine when to exit program
    char **prev = NULL; // holder of previous args
    size_t prevI = 0;   // holder of previous args length

    // loop until exit condition is met
    while (should_run)
    {
        // setting a return point
        if (setjmp(sh_start) != 0)
        {
            printf("\n");
        }

        printf("smash$ "); //Prompt string
        fflush(stdout);

        char *raw;
        size_t i = 0;
        raw = malloc(MAX_LINE_LENGTH); // memory allocation
        args = malloc(MAX_TOKENS);     // memory allocation

        read(STDIN_FILENO, raw, MAX_LINE_LENGTH);
        int len = strlen(raw);
        int amp = 0;

        if (len > 0 && raw[len - 1] == '\n')
        {
            raw[len - 1] = '\0';
            len -= 1;
        }

        // Checking for ampersand
        if (raw[len - 1] == '&')
        {
            amp = 1;
            raw[len - 1] = '\0';
            len -= 1;
        }

        // Checking if we need to execute the last command
        if (strcmp(raw, "!!") == 0)
        {
            // Is there a last command?
            if (prev == NULL)
            {
                printf("No previous command in history");
                longjmp(sh_start, EXIT_FAILURE);
            }

            // Setting args to be the last command
            args = prev;
            i = prevI;
        }
        else
        {
            // tokenizing the args
            char *token = strtok(raw, " ");

            while (token != NULL)
            {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
        }

        // If the args is not exit then call the execute command function
        if (strcmp(args[0], "exit") != 0)
        {
            executecommand(args, amp, i);
        }

        // Else exit the loop and the program
        else
        {
            should_run = 0;
        }

        // If we are looping then save the command
        prev = args;
        prevI = i;
    }
    return 0;
}
