

#include "slow_functions.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
// How nice of me to include a global that tells you how many commands there
// were :)
int total_commands = 0;

// flag for current number of commands
int current_count = 0;


// flag writing
int isWritten = 0;

// intialising a lock
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// intialising the conditions
pthread_cond_t write_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;

// ## Please write some code in the following two functions
void *writer(void *in_ptr)
{

    // cast the parameter to be a char
    char **commands = (char **)in_ptr;

    // while we havent done all the commands
    while (current_count < total_commands)
    {
        // give bad write the mutex
        pthread_mutex_lock(&lock);

        // while something has been written
        while (isWritten)
        {
            // wait for the write condition, waiting for the data to be read
            pthread_cond_wait(&write_cond, &lock);
        }

        // write the command to the buffer
        bad_write(commands[current_count]);

        // increment the number of commands
        current_count++;
        // set flag
        isWritten = 1;

        // Signal the reader that data has been written
        pthread_cond_signal(&read_cond);
        // Unlock the mutex after reading is done
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
void *reader(void *empty)
{
    int count_flag = 0;

    // while true then break
    while (count_flag == 0)
    {

        // give read the mutex
        pthread_mutex_lock(&lock);

        //while writing
        while (!isWritten)
        {
            // wait for the signal from write
            pthread_cond_wait(&read_cond, &lock);
        }

        // call bad read
        bad_read(NULL);

        //set flag
        isWritten = 0;

        // send signal to writer
        pthread_cond_signal(&write_cond);
        // unlock the mutex
        pthread_mutex_unlock(&lock);

        //break when no more commands left to read
        if (current_count >= total_commands)
        {
            count_flag = 1;
        }
    }
    return NULL;
}

int main()
{
    // printf("running");
    //  ## SOME SPACE IN CASE YOU NEED TO INITIALISE VARIABLES
    //

    // ## DO NOT MODIFY BELOW HERE
    // ## Parse STDIN and read into commands
    char *commands[100];
    char line[256];
    while (fgets(line, 256, stdin))
    {
        commands[total_commands] = (char *)(malloc(strlen(line) *
                                                   sizeof(char)));
        strcpy(commands[total_commands], line);
        total_commands = total_commands + 1;
    }
    pthread_t write1;
    pthread_t read1;
    // Creates a thread which executes writer!
    if (pthread_create(&write1, NULL, writer, commands))
    {
        fprintf(stderr, "Error creating write thread\n");
        return 1;
    }
    // Creates a thread which executes reader!
    if (pthread_create(&read1, NULL, reader, NULL))
    {
        fprintf(stderr, "Error creating read thread\n");
        return 1;
    }
    // Ensure Threads Join Well
    if (pthread_join(write1, NULL))
    {
        fprintf(stderr, "Error joining write thread\n");
        return 2;
    }
    if (pthread_join(read1, NULL))
    {
        fprintf(stderr, "Error joining read thread\n");
        return 2;
    }
    // Did you really call the right functions?
    get_output();
    // I am a responsible memory user!
    int command_i;
    for (command_i = 0; command_i < total_commands; command_i++)
    {
        free(commands[command_i]);
    }
    return 0;
    // ## DO NOT MODIFY ABOVE HERE
    //
    // ################################################################################
}
