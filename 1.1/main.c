#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;

/*Simple function to iteratively increment a shared integer 5 times*/
void *increment(void *num) {
    int *val = (int *)num;
    for (int i = 0; i < 100; i++) {
        *val += 1;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("------------Starting main-------------\n");
    long thread;
    pthread_t *thread_handle = NULL;

    // Receiving number of threads from command line
    thread_count = strtol(argv[1], NULL, 10);

    int shared = 0;
    printf("Starting value of shared variable is: %d\n", shared);
    int *ptr = &shared;

    // Allocating memory for thread data
    thread_handle = malloc(thread_count * sizeof(pthread_t));

    // Creating threads
    for (thread = 0; thread < thread_count; thread++) {
        int res = pthread_create(&thread_handle[thread], NULL, increment, ptr);
        if (res) {
            printf("pthread_create error: %d \n", res);
            break;
        }
    }

    // Joining all thread after process completion
    for (thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handle[thread], NULL);
    }

    // Clearing allocated memory
    free(thread_handle);
    thread_handle = NULL;
    ptr = NULL;

    // Expected final value is 20, if we have 4 threads incrementing the value 5 times each
    printf("Final value of variable is: %d\n", shared);
    printf("----------Shutting down main----------\n");

    return 0;
}
