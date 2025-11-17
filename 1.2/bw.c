#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;
long flag; // Flag used for busy-waiting implementation

// Structure which allows for parsing multiple arguments into a thread function
struct arg_struct {
    long *value_ptr;
    long thread_index;
};

/*Simple function to iteratively increment a shared integer one million times*/
void *increment(void *arg) {
    long *val = ((struct arg_struct *)arg)->value_ptr;
    long my_index = ((struct arg_struct *)arg)->thread_index;

    while (flag != my_index)
        ;

    for (long i = 0; i < 1000000; i++) {
        *val += 1;
    }

    flag = (flag + 1) % thread_count;
    return NULL;
}

/*This implementation results in a deterministic value on the "shared" variable*/
int main(int argc, char *argv[]) {
    printf("------------Starting main-------------\n");
    pthread_t *thread_handle = NULL;

    long shared = 0; // Shared variable to be incremented by the thread function
    printf("Starting value of shared variable is: %ld\n", shared);

    long flag = 0;     // Initializing flag as 0, so that thread with index 0 begins execution
    long index = flag; // Index assigned to each thread for differentiation, initialized as 0

    // Receiving number of threads from command line
    thread_count = strtol(argv[1], NULL, 10);

    // Initializing list of arguments to be passed into the thread function
    struct arg_struct **args = malloc(thread_count * sizeof(struct arg_struct *));

    for (int i = 0; i < thread_count; i++) {
        args[i] = malloc(sizeof(long *) + sizeof(long));
    }

    // Allocating memory for thread data
    thread_handle = malloc(thread_count * sizeof(pthread_t));

    // Creating threads
    for (index = 0; index < thread_count; index++) {

        *args[index] = (struct arg_struct){&shared, index};

        if (pthread_create(&thread_handle[index], NULL, increment, (void *)args[index])) {
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

    // Expected deterministic value is 4000000, if we have 4 threads incrementing the value 1000000 times each
    printf("Final value of variable is: %ld\n", shared);
    printf("----------Shutting down main----------\n");

    return 0;
}
