#include <errno.h>
#include <string.h>

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct thread_arguments {
    int **list_of_accounts;  // Pointer to the list used by all threads
    int number_of_transfers; // Number of transfers specific to a thread
    long int thread_rank;
};

pthread_mutex_t mutex; // Mutex used for mutual exclusion lock implementation

int transactions_count;

int remaining_transfers;
int remaining_reads;
int balance_count;

int rand_from_range(int range) {
    int result = rand() % range;
    return result;
}

void *transfer(void *arg) {
    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int transfers = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    // Loop for transactions
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < transfers; i++) {
        // Pick a balance randomly to remove a random amount from
        int sender = rand_from_range(balance_count);
        // printf("Sender is: %d, with balance: %d\n", sender, list[sender]);

        // Pick another balance randomly to add the random amount to, make sure it's not the same as the first
        int receiver = rand_from_range(balance_count);
        while (receiver == sender) {
            receiver = rand_from_range(balance_count);
        }
        // printf("Receiver is: %d, with balance: %d\n", receiver, list[receiver]);

        // Pick a random amount to remove from sender and add to receiver
        int transfer_amount = rand_from_range(100);
        // printf("Amount to transfer is: %d\n", transfer_amount);

        list[sender] = list[sender] - transfer_amount;
        list[receiver] = list[receiver] + transfer_amount;

        printf("Thread: %ld removed %d from sender: %d and added to receiver: %d\n", my_rank, transfer_amount, sender, receiver);
        // printf("Sender balance is: %d\n", list[sender]);
        // printf("Receiver balance is: %d\n", list[receiver]);
    }
    pthread_mutex_unlock(&mutex);
    free(argstruct);
    return NULL;
}

// TODO: Implement reading functionality
// BUG: Lock functionality required
void *read(void *arg) {

    printf("read works\n");
    return NULL;
}

double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// TODO: Implement functionality for different types of locks
int main(int argc, char *argv[]) {
    // Timespec initialization
    struct timespec parallel_execution_start, parallel_execution_finish;

    // Mutex initialization
    pthread_mutex_init(&mutex, NULL);

    // Receiving program arguments
    balance_count = atoi(argv[1]);
    transactions_count = atoi(argv[2]);
    long read_percentage = strtol(argv[3], NULL, 10);
    char *lock_type = argv[4];
    printf("Lock type: %s\n", lock_type);
    long thread_count = strtol(argv[5], NULL, 10);
    long thread = 0;

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

    // TODO: Use the total number of transactions and the percentage of reading transactions to calculate the exact number of reading transactions,
    // subtract that from the total to calculate the number of writing transactions,
    // use these numbers to distribute threads efficiently(the more a type of transactions is used, the more threads should be allocated for it),
    // maybe number_of_allocated_threads = total_number_of_threads * (number_of_writing_transactions / total_number_of_transactions)
    // and number_of_allocated_threads = total_number_of_threads * (number_of_reading_transactions / total_number_of_transactions)
    // then for the number_of_allocated_threads, each one gets an equal number of transactions to perform

    // Calculating number of reading transactions based on the total number of transactions and percentage of reading transactions
    // Using the ceil() function in cases where the calculation leads to a non-integer value, so that most transactions end up as reading ones
    int read_transactions_count = ceil(((read_percentage / (float)100) * transactions_count));
    printf("Number of reading transactions: %d\n", read_transactions_count);

    // Number of writing transactions is just however many transactions remain from the total
    int write_transactions_count = transactions_count - read_transactions_count;
    printf("Number of writing transactions: %d\n", write_transactions_count);

    // Calculating number of reading threads based on the given percentage of reading transactions
    double reader_calc = ((read_percentage / (float)100) * thread_count);
    long reader_threads = lround(reader_calc);

    long transfer_threads = thread_count - reader_threads;

    printf("Writers are: %ld, Readers are: %ld\n", transfer_threads, reader_threads);

    pthread_t *writer_handle = malloc(transfer_threads * sizeof(pthread_t));
    pthread_t *reader_handle = malloc(reader_threads * sizeof(pthread_t));

    // Calculate maximum number of transactions a thread gets to perform
    int transactions_per_transfer_thread = ceil(write_transactions_count / (double)transfer_threads);
    printf("transactions_per_transfer_thread is: %d\n", transactions_per_transfer_thread);
    remaining_transfers = write_transactions_count;

    timespec_get(&parallel_execution_start, TIME_UTC);

    // Initializing writer threads
    // TODO: Pass transactions_per_transfer_thread as an argument into each thread function, along with the balance_list in a structure
    // retain a counter of remaining transfers
    for (thread = 0; thread < transfer_threads; thread++) {
        if (remaining_transfers < transactions_per_transfer_thread) {
            transactions_per_transfer_thread = remaining_transfers;
        }

        struct thread_arguments *args = (struct thread_arguments *)malloc(sizeof(struct thread_arguments));

        args->list_of_accounts = &balance_list;
        args->number_of_transfers = transactions_per_transfer_thread;
        args->thread_rank = thread;

        printf("Creating thread: %ld\n", thread);
        int create_res = pthread_create(&writer_handle[thread], NULL, transfer, (void *)args);
        if (create_res != 0) {
            printf("Thread creation error for thread %ld: %d(%s)\n", thread, create_res, strerror(create_res));
            break;
        }
        remaining_transfers -= transactions_per_transfer_thread;
    }

    // Initializing reader threads
    for (thread = 0; thread < reader_threads; thread++) {
        pthread_create(&reader_handle[thread], NULL, read, (void *)&balance_list);
    }

    // Joining writer threads
    for (thread = 0; thread < transfer_threads; thread++) {
        pthread_join(writer_handle[thread], NULL);
    }

    // Joining reader threads
    for (thread = 0; thread < reader_threads; thread++) {
        pthread_join(reader_handle[thread], NULL);
    }

    timespec_get(&parallel_execution_finish, TIME_UTC);

    double total_timespec = time_elapsed(parallel_execution_start, parallel_execution_finish);

    printf("Total timespec of parallel execution was: %lf\n", total_timespec);

    printf("New list is: \n");
    printf("[");
    for (int i = 0; i < balance_count; i++) {
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n");

    free(balance_list);

    return 0;
}
