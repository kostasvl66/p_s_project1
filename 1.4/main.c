#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

long thread_count;
int transactions_count;

void *write(void *arg) {
    printf("write works\n");
    return NULL;
}

void *read(void *arg) {
    printf("read works\n");
    return NULL;
}

// TODO: Implement functionality for different types of locks
int main(int argc, char *argv[]) {
    int balance_count = atoi(argv[1]);
    transactions_count = atoi(argv[2]);
    long read_percentage = atol(argv[3]);
    char *lock_type = argv[4];
    thread_count = atol(argv[5]);

    // Initializing list with 5 balance accounts
    int *balance_list = malloc(balance_count * sizeof(int));

    // Adding random values to list
    printf("List should be: \n");
    printf("[");
    for (int i = 0; i < 5; i++) {
        balance_list[i] = rand();
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n");

    // Calculating number of reading threads based on the given percentage of reading transactions
    double reader_calc = ((read_percentage / (float)100) * thread_count);
    long reader_count = lround(reader_calc);

    long writer_count = thread_count - reader_count;

    pthread_t *writer_handle = malloc(writer_count * sizeof(pthread_t));
    pthread_t *reader_handle = malloc(reader_count * sizeof(pthread_t));

    // Initializing writer threads
    for (long thread = 0; thread < writer_count; thread++) {
        pthread_create(&writer_handle[thread], NULL, write, (void *)&balance_list);
    }

    // Initializing reader threads
    for (long thread = 0; thread < reader_count; thread++) {
        pthread_create(&reader_handle[thread], NULL, read, (void *)&balance_list);
    }

    // Joining writer threads
    for (long thread = 0; thread < writer_count; thread++) {
        pthread_join(writer_handle[thread], NULL);
    }

    // Joining reader threads
    for (long thread = 0; thread < reader_count; thread++) {
        pthread_join(reader_handle[thread], NULL);
    }

    return 0;
}
