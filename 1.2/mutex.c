#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;
pthread_mutex_t mutex; // Mutex used for mutual exclusion lock implementation

// Structure which allows for parsing multiple arguments into a thread function
struct arg_struct {
    long *value_ptr;
    long thread_index;
};

/*Simple function to iteratively increment a shared integer one million times*/
void *increment(void *arg) {
    long *val = (long *)arg;

    pthread_mutex_lock(&mutex);

    for (long i = 0; i < 1000000; i++) {
        *val += 1;
    }

    pthread_mutex_unlock(&mutex);

    val = NULL;
    return NULL;
}

/*This implementation results in a deterministic value on the "shared" variable*/
/*A simple mutex lock is used to ensure conflicts are avoided*/
int main(int argc, char *argv[]) {
    printf("------------Starting main-------------\n");
    pthread_t *thread_handle = NULL;

    long shared = 0; // Shared variable to be incremented by the thread function
    printf("Starting value of shared variable is: %ld\n", shared);

    pthread_mutex_init(&mutex, NULL); // Initializing mutex
    long index = 0;                   // Index assigned to each thread for differentiation, initialized as 0

    thread_count = strtol(argv[1], NULL, 10);                 // Receiving number of threads from command line
    thread_handle = malloc(thread_count * sizeof(pthread_t)); // Allocating memory for thread data

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

    // Clearing allocated memory
    free(thread_handle);
    thread_handle = NULL;
    pthread_mutex_destroy(&mutex);

    printf("Final value of variable is: %ld\n", shared);
    printf("----------Shutting down main----------\n\n");

    return 0;
}
