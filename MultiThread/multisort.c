#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

/*
This sorting program brought to you by: Aaron Mechanic
CS 340 Programming project 2
Multithreaded sorting program

**Commented out code lines are likely debug. Please ignore them.
*/

#define MAX_LENGTH 100
int input[MAX_LENGTH];  //raw unsorted array
int sorted[MAX_LENGTH]; //final sorted array

//Load order for methods matters. Declare all methods before main.

void printdbg(char *str) //((Debugging))
{
    // Simple printf statement and fflush so it always outputs even if the program crashes
    static int num = 0;
    fprintf(stderr, "%d: %s\n", num++, str);
    fflush(stdout);
}

void *sortIn(void *param) //this is the sorting method for the halves
{
    int *points = (int *)param; //pointers...
    int temp;                   //temp int for swapping
    int start = points[0];      //starting point in the main array
    int end = points[1];        //ending point in the main array
    // printf("start %d\nEnd %d\n", start, end);
    for (int q = start; q < end; q++)
    {
        for (int k = start; k < end; k++) //nested for loops because I can only do brute force coding
        {
            if (input[q] < input[k])
            {
                // printf("was: %d\n", input[q]);
                temp = input[q];
                input[q] = input[k]; //swap everything
                input[k] = temp;
                // printf("now: %d\n", input[q]);
            }
        }
    }
}

void *merge(void *merge) //this is the merging method
{
    int *points = (int *)merge;
    int end = points[1];
    int counter = 0;
    int a = 0;         //start of the input aka half1
    int b = points[0]; //midpoint of the input aka half2

    while (counter < end) //counting up to the size of the input
    {
        if (a == points[0]) //if we've reached the end of the first half
        {
            sorted[counter] = input[b]; //we can just add all of b in order
            b++;
        }
        else if (b == end) //same as above
        {
            sorted[counter] = input[a];
            a++;
        }
        else if (input[a] <= input[b]) //if the number in half 1 is less than the number in half 2 or equal
        {
            // printf("At %d A is %d, B is %d\n", counter, input[a], input[b]);
            sorted[counter] = input[a]; //then add the half 1 number to sorted array
            a++;                        //and move to the next number in half 2
        }
        else //if half 2 number is smaller
        {
            // printf("At %d B is %d, A is %d\n", counter, input[b], input[a]);
            sorted[counter] = input[b]; //add half 2 number to array
            b++;                        //and move to next half 2
        }
        counter++; //next position in array
    }
}

int main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "r"); //open the file
    if (file == NULL)
    {
        printf("No file"); //no file, no work
        return 0;          //rage quit
    }
    int n = 0;
    while (fscanf(file, "%d", &input[n]) == 1) //read the file
    {
        //printf("%d\n", input[n]); //making sure this loop works
        n++;
    }
    fclose(file);                                            //Be nice, close the file when done with it
    int inputsize = 0;                                       //Dynamic arrays are hard so we do this the me way
    while (input[inputsize] > 0 || input[(inputsize + 1)] > 0) //finding the real size of the input (if less than 100)
    {
        inputsize++;
    }
    printf("Input size: %d\n", inputsize);

    int points[2];
    pthread_t thread1, thread2, thread3;            //create thread ids
    points[0] = 0;                                  //beginning for thread 1
    points[1] = (inputsize / 2);                    //end point for thread 1
    pthread_create(&thread1, NULL, sortIn, points); //create the threads
    pthread_join(thread1, NULL);                    //dont forget to join your threads

    points[0] = (inputsize / 2);                    //beginning for thread 2
    points[1] = inputsize;                          //end for thread 2
    pthread_create(&thread2, NULL, sortIn, points); //create thread 2
    pthread_join(thread2, NULL);                    //you will get weird errors otherwise

    pthread_create(&thread3, NULL, merge, points); //merge thread
    pthread_join(thread3, NULL);

    for (int x = 0; x < inputsize; x++) //output
    {
        printf("%d: %d\n", x + 1, sorted[x]);
    }

    fflush(stdout); //just in case
    return 0;
}