#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
// ## PART 1
// Mutex and flag to avoid modification
static int anti_cheat = 0;
static pthread_mutex_t lock;
// Sleeps for one second, increments a variable
// Checks if the variable is above 9
// Then gets grumpy if it is above 10
void *slow_function1(void *empty)
{
    static int special = 0;
    sleep(1);
    pthread_mutex_lock(&lock);
    if (anti_cheat == 1)
    {
        special = special + 1;
    }
    pthread_mutex_unlock(&lock);
    if (special > 9)
    {
        printf("Work Done\n");
    }
    sleep(1);
    if (special > 10)
    {
        printf("FAIL!!\n");
    }
    return NULL;
}
// Initialises a mutex
// Initiates a timer (2 seconds)
// Work must be done before this function finishes
void *slow_function2(void *empty)
{
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("FAIL!\n");
    }
    printf("Start 2\n");
    anti_cheat = 1;
    sleep(2);
    printf("End 2\n");
}
// ## PART 2
static char buffer[100];
static int write_calls = 0;
static int read_calls = 0;
static int written = 0;
void *bad_write(void *input)
{
    //printf("writing");
    sleep(0.01);
    strcpy(buffer, (char *)input);
    sleep(0.01);
    written = 1;
    sleep(0.01);
    write_calls = write_calls + 1;
    sleep(0.01);
}
void *bad_read(void *empty)
{
    //printf("reading");
    sleep(0.01);
    printf("%s", buffer);
    sleep(0.01);
    written = 0;
    sleep(0.01);
    strcpy(buffer, "Fail!!\n");
    sleep(0.01);
    read_calls = read_calls + 1;
    sleep(0.01);
}
int get_written()
{
    return written;
}
void get_output()
{
    printf("Read: %d\n", read_calls);
    printf("Written: %d\n", write_calls);
}
