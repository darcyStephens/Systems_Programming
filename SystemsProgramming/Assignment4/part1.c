#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "slow_functions.h"

int main()
{
    //tutors please ignore the following comments, I am writing out how joining threads works
    //to help me when doing this question

    //you make a thread, it runs code, you can multiple threads to run the same bit of code
    //this main file in run on a thread, when you call new threads, join them back to the main thread
    //when calling pthread_join() thread waits until the thread your joining finishes.
    //main thread then continues its execution

    //ok tutors welcome back

    //create 10 threads to run 10 instances of slow_function1.
    pthread_t slow1_threads[10];

    pthread_t slow2_threads;
    //creating a thread to run slow_function2
    if (pthread_create(&slow2_threads, NULL, slow_function2, NULL) != 0)
    {
        printf("error creating slow2 thread");
        return 1;
    }

    
    for (int i = 0; i < 10; i++)
    {

        if (pthread_create(&slow1_threads[i], NULL, slow_function1, NULL) != 0)
        {
            printf("error creating slow1 thread");
            return 1;
        }
    }

    //join created threads running slow_function1 back to the main thread
    for (int i = 0; i < 10; i++)
    {

        if (pthread_join(slow1_threads[i], NULL) != 0)
        {
            printf("error joining slow1 thread");
            return 1;
        }
    }
    //join created thread running slow_function2 back to the main thread
    if(pthread_join(slow2_threads, NULL)!=0)
    {
        printf("error joining slow2");
        return 1;
    }
    return 0;
}