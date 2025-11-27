#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;
pthread_rwlock_t rwlock;

/*Simple function to iteratively increment a shared integer one million times*/
void *increment(void *num) {
    long *val = (long *)num;
    pthread_rwlock_wrlock(&rwlock);
    for (long i = 0; i < 1000000; i++) {
        *val += 1;
    }
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

/*Calculates the time elapsed between two instances of the timespec structure*/
double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

/*This implementation results in a non-deterministic value on the "shared" variable*/
int main(int argc, char *argv[]) {
    printf("------------Starting rw-------------\n");
    struct timespec execution_start, execution_finish;
    long thread;
    pthread_t *thread_handle = NULL;

    // Receiving number of threads from command line
    thread_count = strtol(argv[1], NULL, 10);

    long shared = 0;
    printf("Starting value of shared variable is: %ld\n", shared);
    pthread_rwlock_init(&rwlock, NULL);

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
    pthread_rwlock_destroy(&rwlock);

    // Get the finishing time of execution
    timespec_get(&execution_finish, TIME_UTC);

    // Calculate total tine of execution
    double execution_time = time_elapsed(execution_start, execution_finish);

    // Clearing allocated memory
    free(thread_handle);
    thread_handle = NULL;

    // Expected deterministic value is 4000000, if we have 4 threads incrementing the value 1000000 times each
    printf("Final value of variable is: %ld\n", shared);
    printf("Time of execution is: %lf\n", execution_time);
    printf("----------Shutting down rw----------\n\n");

    return 0;
}
