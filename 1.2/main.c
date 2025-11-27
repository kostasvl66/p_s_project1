#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int thread_count;

/*Simple function to iteratively increment a shared integer one million times*/
void *increment(void *num) {
    long *val = (long *)num;
    for (long i = 0; i < 1000000; i++) {
        *val += 1;
    }
    return NULL;
}

/*Calculates the time elapsed between two instances of the timespec structure*/
double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

/*This implementation results in a non-deterministic value on the "shared" variable*/
int main(int argc, char *argv[]) {
    printf("------------Starting main-------------\n");
    struct timespec execution_start, execution_finish;
    long thread;
    pthread_t *thread_handle = NULL;

    // Receiving number of threads from command line
    thread_count = strtol(argv[1], NULL, 10);

    long shared = 0;
    printf("Starting value of shared variable is: %ld\n", shared);

    // Allocating memory for thread data
    thread_handle = malloc(thread_count * sizeof(pthread_t));

    // Get the starting time of execution
    timespec_get(&execution_start, TIME_UTC);

    // Creating threads
    for (thread = 0; thread < thread_count; thread++) {
        int res = pthread_create(&thread_handle[thread], NULL, increment, (void *)&shared);
        if (res) {
            printf("pthread_create error: %d \n", res);
            break;
        }
    }

    // Joining all threads after process completion
    for (thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handle[thread], NULL);
    }

    // Get the finishing time of execution
    timespec_get(&execution_finish, TIME_UTC);

    // Calculate total tine of execution
    double execution_time = time_elapsed(execution_start, execution_finish);

    // Clearing allocated memory
    free(thread_handle);
    thread_handle = NULL;

    // Printing value after all increments are finished
    printf("Final value of variable is: %ld\n", shared);
    printf("Time of execution is: %lf\n", execution_time);
    printf("----------Shutting down main----------\n\n");

    return 0;
}
