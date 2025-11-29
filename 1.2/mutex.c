#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;
long increment_count;
pthread_mutex_t mutex; // Mutex used for mutual exclusion lock implementation

/*Simple function to iteratively increment a shared integer one million times*/
void *increment(void *arg) {
    long *val = (long *)arg;

    pthread_mutex_lock(&mutex);

    for (long i = 0; i < increment_count; i++) {
        *val += 1;
    }

    pthread_mutex_unlock(&mutex);

    val = NULL;
    return NULL;
}

/*Calculates the time elapsed between two instances of the timespec structure*/
double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

/*This implementation results in a deterministic value on the "shared" variable*/
/*A simple mutex lock is used to ensure conflicts are avoided*/
int main(int argc, char *argv[]) {
    printf("------------Starting mutex-------------\n");
    FILE *fileptr;
    struct timespec execution_start, execution_finish;
    pthread_t *thread_handle = NULL;

    // Opening file for storing execution time
    fileptr = fopen("results.txt", "a");
    if (fileptr == NULL) {
        perror("File could not be opened");
    }

    long shared = 0; // Shared variable to be incremented by the thread function
    printf("Starting value of shared variable is: %ld\n", shared);

    pthread_mutex_init(&mutex, NULL); // Initializing mutex
    long index = 0;                   // Index assigned to each thread for differentiation, initialized as 0

    thread_count = strtol(argv[1], NULL, 10); // Receiving number of threads from command line
    increment_count = strtol(argv[2], NULL, 10);
    thread_handle = malloc(thread_count * sizeof(pthread_t)); // Allocating memory for thread data

    // Get the starting time of execution
    timespec_get(&execution_start, TIME_UTC);

    // Creating threads
    for (index = 0; index < thread_count; index++) {
        if (pthread_create(&thread_handle[index], NULL, increment, (void *)&shared)) {
            perror("Error while creating thread");
        }
    }

    // Joining all threads after process completion
    for (index = 0; index < thread_count; index++) {
        pthread_join(thread_handle[index], NULL);
    }

    // Get the finishing time of execution
    timespec_get(&execution_finish, TIME_UTC);

    // Calculate total tine of execution
    double execution_time = time_elapsed(execution_start, execution_finish);

    // Writing final execution time to the "results.txt file"
    fprintf(fileptr, "%lf\n", execution_time);

    // Clearing allocated memory
    free(thread_handle);
    thread_handle = NULL;
    pthread_mutex_destroy(&mutex);
    fclose(fileptr);

    printf("Final value of variable is: %ld\n", shared);
    printf("Time of execution is: %lf\n", execution_time);
    printf("----------Shutting down mutex----------\n\n");

    return 0;
}
