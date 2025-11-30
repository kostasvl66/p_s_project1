#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct thread_arguments {
    int **list_of_accounts;  // Pointer to the list used by all threads
    int number_of_transfers; // Number of transfers specific to a thread
    long int thread_rank;
};

pthread_mutex_t mutex;   // Mutex used for mutual exclusion lock implementation
pthread_rwlock_t rwlock; // RWlock used for Readers-Writers lock implementation
char *lock_type;         // String used to determine the type of lock which must be used during execution
                         // Accepted values are "MUTEX_COARSEGRAINED"/"MUTEX_FINEGRAINED" and "RW_COARSEGRAINED"/"RW_FINEGRAINED"

/*Function for initializing a lock, while determining if it's a mutex or a readers-writer lock*/
int mtxrw_init(char *lock, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // Mutex initialization
        return pthread_mutex_init(mtx, NULL);
    } else if (!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) {
        // RWlock initialization
        return pthread_rwlock_init(rw, NULL);
    }
    return -1;
}

/*Function for locking a mutex or readers-writer lock*/
int mtxrw_lock(char *lock, char *rw_type, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // Mutex locking
        return pthread_mutex_lock(mtx);
    } else if ((!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) && !strcmp(rw_type, "writer")) {
        // Writer lock locking
        return pthread_rwlock_wrlock(rw);
    } else if ((!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) && !strcmp(rw_type, "reader")) {
        // Reader lock locking
        return pthread_rwlock_rdlock(rw);
    }
    return -1;
}

/*Function for unlocking a mutex or readers-writer lock*/
int mtxrw_unlock(char *lock, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // Mutex unlocking
        return pthread_mutex_unlock(mtx);
    } else if (!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) {
        // RWlock unlocking
        return pthread_rwlock_unlock(rw);
    }
    return -1;
}

int balance_count; // Size of the list of balances used for dynamic allocation

// Lists of locks used for finegrained locking
pthread_mutex_t *mutex_list = NULL;
pthread_rwlock_t *rw_list = NULL;

/*Returns a random number in the range 0 - <range_max>*/
int rand_from_range(int range_max) {
    int result = rand() % range_max;
    return result;
}

signed int *thread_actions;

/*Thread function for performing transfer transactions*/
void *balance_transfer(void *arg) {
    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int transfers = argstruct->number_of_transfers;
    // long my_rank = argstruct->thread_rank;
    // Loop for transactions
    for (int i = 0; i < transfers; i++) {
        // Pick a random amount to remove from sender and add to receiver
        int transfer_amount = rand_from_range(100);

        // Pick a balance randomly to remove a random amount from
        int sender = rand_from_range(balance_count);

        // Pick another balance randomly to add the random amount to, make sure it's not the same as the first
        int receiver = rand_from_range(balance_count);
        while (receiver == sender) {
            receiver = rand_from_range(balance_count);
        }

        // Using lock on critical section
        // Deciding which lock structure to use(lock object or list of lock objects) based on the lock type chosen by the user
        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_lock(lock_type, "writer", &mutex_list[sender], &rw_list[sender]);
        } else {
            mtxrw_lock(lock_type, "writer", &mutex, &rwlock);
        }
        thread_actions[sender] -= transfer_amount;
        list[sender] = list[sender] - transfer_amount;
        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_unlock(lock_type, &mutex_list[sender], &rw_list[sender]);
        } else {
            mtxrw_unlock(lock_type, &mutex, &rwlock);
        }

        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_lock(lock_type, "writer", &mutex_list[receiver], &rw_list[receiver]);
        } else {
            mtxrw_lock(lock_type, "writer", &mutex, &rwlock);
        }
        thread_actions[receiver] += transfer_amount;
        list[receiver] = list[receiver] + transfer_amount;
        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_unlock(lock_type, &mutex_list[receiver], &rw_list[receiver]);
        } else {
            mtxrw_unlock(lock_type, &mutex, &rwlock);
        }

        // printf("Transfer thread: %ld removed %d from sender: %d and added to receiver: %d\n", my_rank, transfer_amount, sender, receiver);
    }
    return NULL;
}

/*Thread function for performing reading/printing transactions*/
void *balance_read(void *arg) {
    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int reads = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    long balance_sum = 0;

    // Loop for transactions
    for (int i = 0; i < reads; i++) {
        // Pick a balance randomly to read its amount
        int bal = rand_from_range(balance_count);

        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_lock(lock_type, "reader", &mutex_list[bal], &rw_list[bal]);
        } else {
            mtxrw_lock(lock_type, "reader", &mutex, &rwlock);
        }
        balance_sum += list[bal];
        // sleep(1);
        if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
            mtxrw_unlock(lock_type, &mutex_list[bal], &rw_list[bal]);
        } else {
            mtxrw_unlock(lock_type, &mutex, &rwlock);
        }

        // printf("Read balance: %d, of account: %d\n", list[bal], bal);
    }
    printf("Read thread: %ld, Sum of read balances is: %ld\n", my_rank, balance_sum);

    return NULL;
}

double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    // Timespec initialization
    struct timespec parallel_execution_start, parallel_execution_finish;

    // Receiving program arguments
    if (argc != 6) {
        perror("Too few arguments given at execution");
    }
    balance_count = atoi(argv[1]);
    int transactions_count = atoi(argv[2]);           // Number of transactions for each thread to perform
    long read_percentage = strtol(argv[3], NULL, 10); // Percentage of total transactions to be used for reading
    lock_type = argv[4];
    long thread_count = strtol(argv[5], NULL, 10);

    // Checking and initializing lock type
    printf("Lock type: %s\n", lock_type);
    int res = mtxrw_init(lock_type, &mutex, &rwlock);
    if (res != 0) {
        printf("Result of init is: %d", res);
    }

    // Initializing list of locks
    if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {
        mutex_list = malloc(balance_count * sizeof(pthread_mutex_t));
        rw_list = malloc(balance_count * sizeof(pthread_rwlock_t));
        for (int k = 0; k < balance_count; k++) {
            mtxrw_init(lock_type, &mutex_list[k], &rw_list[k]);
        }
    }

    // Initializing list with 5 balance accounts
    int *balance_list = malloc(balance_count * sizeof(int));

    // Initializing list used to check for proper thread execution
    // Every action(adding or removing) performed on a balance is summed based on said balance's index
    thread_actions = malloc(balance_count * sizeof(int));

    // Initializing list to use with the thread actions list in order to check for proper thread execution
    int *expected_list = malloc(balance_count * sizeof(int));

    // Adding random values to list
    printf("Starting list is: \n");
    printf("[");
    for (int i = 0; i < balance_count; i++) {
        balance_list[i] = rand_from_range(1000);
        printf("\t%d,", balance_list[i]);

        // Adding all values to the expected list for later checking
        expected_list[i] = balance_list[i];

        // Setting all indexes to 0 for accurate summation of all actions
        thread_actions[i] = 0;
    }
    printf("\t]\n\n");

    // Calculating total transaction for later use
    int total_transactions = transactions_count * thread_count;

    // Calculating number of reading transactions based on the total number of transactions and percentage of reading transactions
    // Using the ceil() function in cases where the calculation leads to a non-integer value, so that most transactions end up as reading ones
    int read_transactions_count = ceil(((read_percentage / (float)100) * total_transactions));
    // printf("Number of reading transactions: %d\n", read_transactions_count);

    // Number of writing transactions is just however many transactions remain from the total
    // int write_transactions_count = total_transactions - read_transactions_count;
    // printf("Number of writing transactions: %d\n", write_transactions_count);

    // All but one thread are used for reading operations
    long reader_threads = thread_count - 1;

    // One thread is used for writing operations
    // long transfer_threads = 1;

    // printf("Writers are: %ld, Readers are: %ld\n", transfer_threads, reader_threads);

    pthread_t *reader_handle = malloc(reader_threads * sizeof(pthread_t));
    pthread_t writer_handle;

    int transactions_per_read_thread = ceil(read_transactions_count / (double)reader_threads);
    // printf("transactions_per_read_thread is: %d\n", transactions_per_read_thread);

    int transactions_per_transfer_thread = total_transactions - transactions_per_read_thread;
    // printf("transactions_per_transfer_thread is: %d\n", transactions_per_transfer_thread);

    int remaining_reads = read_transactions_count;

    timespec_get(&parallel_execution_start, TIME_UTC);

    struct thread_arguments *args = (struct thread_arguments *)malloc(sizeof(struct thread_arguments));

    // Initializing reader threads
    for (long thread = 0; thread < reader_threads; thread++) {
        if (remaining_reads < transactions_per_read_thread) {
            transactions_per_read_thread = remaining_reads;
        }

        args->list_of_accounts = &balance_list;
        args->number_of_transfers = transactions_per_transfer_thread;
        args->thread_rank = thread;

        // printf("Creating read thread: %ld\n", thread);
        pthread_create(&reader_handle[thread], NULL, balance_read, (void *)args);

        remaining_reads -= transactions_per_read_thread;
    }

    // Initializing a single writer thread
    args->list_of_accounts = &balance_list;
    args->number_of_transfers = transactions_per_transfer_thread;
    args->thread_rank = 0;

    // printf("Creating transfer thread\n");
    pthread_create(&writer_handle, NULL, balance_transfer, (void *)args);

    // Joining reader threads
    for (long thread = 0; thread < reader_threads; thread++) {
        pthread_join(reader_handle[thread], NULL);
    }

    // Joining writer thread
    pthread_join(writer_handle, NULL);

    timespec_get(&parallel_execution_finish, TIME_UTC);

    // Evaluating total elapsed time of parallel execution
    double total_timespec = time_elapsed(parallel_execution_start, parallel_execution_finish);

    printf("\nTotal timespec of parallel execution was: %lf\n\n", total_timespec);

    // Printing the list of balances after thread execution is finished
    printf("New list is: \n");
    printf("[");
    for (int i = 0; i < balance_count; i++) {
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n\n");

    // Counter checks how many balances from the balance_list match their counterparts in the expected_list
    int correct_count = 0;

    // Printing the list of the balances expected after thread execution
    printf("Expected list is: \n");
    printf("[");
    for (int i = 0; i < balance_count; i++) {
        expected_list[i] += thread_actions[i];
        printf("\t%d,", expected_list[i]);
        if (expected_list[i] == balance_list[i]) {
            correct_count++;
        }
    }
    printf("\t]\n\n");

    if (correct_count == balance_count) {
        perror("All balances have been transferred from/to as expected");
    } else {
        perror("Not all balances have been transferred from/to as expected");
    }

    // Clearing memory
    pthread_mutex_destroy(&mutex);
    pthread_rwlock_destroy(&rwlock);
    free(balance_list);
    balance_list = NULL;
    free(reader_handle);
    reader_handle = NULL;

    if (!strcmp(lock_type, "MUTEX_FINEGRAINED") || !strcmp(lock_type, "RW_FINEGRAINED")) {

        for (int k = 0; k < balance_count; k++) {
            pthread_mutex_destroy(&mutex_list[k]);
            pthread_rwlock_destroy(&rw_list[k]);
        }
        free(mutex_list);
        mutex_list = NULL;
        free(rw_list);
        rw_list = NULL;
    }

    return 0;
}
