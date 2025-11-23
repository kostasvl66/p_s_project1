#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long thread_count;
int transactions_count;
int balance_count;

int rand_from_range(int range) {
    int result = rand() % range + 1;
    return result;
}

// TODO: Implement transferring functionality
void *transfer(void *arg) {
    int **temp = (int **)arg;
    int *list = *temp;
    // Loop for transactions
    for (int i = 0; i < transactions_count; i++) {
        // Pick a balance randomly to remove a random amount from
        int sender = rand_from_range(balance_count);
        printf("Sender is: %d, with balance: %d\n", sender, list[sender]);

        // Pick another balance randomly to add the random amount to, make sure it's not the same as the first
        int receiver = rand_from_range(balance_count);
        while (receiver == sender) {
            receiver = rand_from_range(balance_count);
        }
        printf("Receiver is: %d, with balance: %d\n", receiver, list[receiver]);

        // Pick a random amount to remove from sender and add to receiver
        int transfer_amount = rand_from_range(100);
        printf("Amount to transfer is: %d\n", transfer_amount);

        list[sender] = list[sender] - transfer_amount;
        list[receiver] = list[receiver] + transfer_amount;

        printf("Removed %d from sender: %d and added to receiver: %d\n", transfer_amount, sender, receiver);
        printf("Sender balance is: %d\n", list[sender]);
        printf("Receiver balance is: %d\n", list[receiver]);
    }
    return NULL;
}

// TODO: Implement reading functionality
void *read(void *arg) {
    printf("read works\n");
    return NULL;
}

// TODO: Implement functionality for different types of locks
int main(int argc, char *argv[]) {
    balance_count = atoi(argv[1]);
    transactions_count = atoi(argv[2]);
    long read_percentage = atol(argv[3]);
    char *lock_type = argv[4];
    printf("Lock type: %s\n", lock_type);
    thread_count = atol(argv[5]);

    // Initializing list with 5 balance accounts
    int *balance_list = malloc(balance_count * sizeof(int));

    // Adding random values to list
    printf("List is: \n");
    printf("[");
    for (int i = 0; i < balance_count; i++) {
        balance_list[i] = rand_from_range(1000);
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n");

    // Calculating number of reading threads based on the given percentage of reading transactions
    double reader_calc = ((read_percentage / (float)100) * thread_count);
    long reader_count = lround(reader_calc);

    long writer_count = thread_count - reader_count;

    printf("Writers are: %ld, Readers are: %ld\n", writer_count, reader_count);

    pthread_t *writer_handle = malloc(writer_count * sizeof(pthread_t));
    pthread_t *reader_handle = malloc(reader_count * sizeof(pthread_t));

    clock_t start_time = clock();

    // Initializing writer threads
    for (long thread = 0; thread < writer_count; thread++) {
        pthread_create(&writer_handle[thread], NULL, transfer, (void *)&balance_list);
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

    clock_t end_time = clock();

    double total_time = (double)(end_time - start_time) / (double)CLOCKS_PER_SEC;

    printf("Total time of parallel execution was: %lf\n", total_time);

    return 0;
}
