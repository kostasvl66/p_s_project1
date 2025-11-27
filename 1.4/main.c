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

pthread_mutex_t mutex;   // Mutex used for mutual exclusion lock implementation
pthread_rwlock_t rwlock; // RWlock used for Readers-Writers lock implementation
char *lock_type;         // String used to determine the type of lock which must be used during execution (Values are "MUTEX" and "RW")

int mtxrw_init(char *lock, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // printf("init mutex\n");
        // Mutex initialization
        return pthread_mutex_init(mtx, NULL);
    } else if (!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) {
        // printf("init rw\n");
        // RWlock initialization
        return pthread_rwlock_init(rw, NULL);
    }
    return -1;
}

int mtxrw_lock(char *lock, char *rw_type, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // printf("lock mutex\n");
        // Mutex locking
        return pthread_mutex_lock(mtx);
    } else if ((!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) && !strcmp(rw_type, "writer")) {
        // printf("lock rw writer\n");
        // Writer lock locking
        return pthread_rwlock_wrlock(rw);
    } else if ((!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) && !strcmp(rw_type, "reader")) {
        // printf("lock rw reader\n");
        // Reader lock locking
        return pthread_rwlock_rdlock(rw);
    }
    return -1;
}

int mtxrw_unlock(char *lock, pthread_mutex_t *mtx, pthread_rwlock_t *rw) {
    if (!strcmp(lock, "MUTEX_COARSEGRAINED") || !strcmp(lock, "MUTEX_FINEGRAINED")) {
        // printf("unlock mutex\n");
        // Mutex unlocking
        return pthread_mutex_unlock(mtx);
    } else if (!strcmp(lock, "RW_COARSEGRAINED") || !strcmp(lock, "RW_FINEGRAINED")) {
        // printf("unlock rw\n");
        // RWlock unlocking
        return pthread_rwlock_unlock(rw);
    }
    return -1;
}

int transactions_count; // Number of transactions for each thread to perform

int balance_count; // Size of the list of balances used for dynamic allocation
pthread_mutex_t *mutex_list;
pthread_rwlock_t *rw_list;

/*Returns a random number in the range 0 - <range_max>*/
int rand_from_range(int range_max) {
    int result = rand() % range_max;
    return result;
}

/*Thread function for performing transfer transactions*/
void *coarse_grained_transfer(void *arg) {
    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int transfers = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    // Loop for transactions
    for (int i = 0; i < transfers; i++) {
        // Pick a random amount to remove from sender and add to receiver
        int transfer_amount = rand_from_range(100);

        // Pick a balance randomly to remove a random amount from
        int sender = rand_from_range(balance_count);
        // printf("Sender is: %d, with balance: %d\n", sender, list[sender]);

        // Pick another balance randomly to add the random amount to, make sure it's not the same as the first
        int receiver = rand_from_range(balance_count);
        while (receiver == sender) {
            receiver = rand_from_range(balance_count);
        }
        // printf("Receiver is: %d, with balance: %d\n", receiver, list[receiver]);

        // pthread_mutex_lock(&mutex);
        // pthread_rwlock_wrlock(&rwlock);
        mtxrw_lock(lock_type, "writer", &mutex, &rwlock);
        list[sender] = list[sender] - transfer_amount;
        list[receiver] = list[receiver] + transfer_amount;
        mtxrw_unlock(lock_type, &mutex, &rwlock);
        // pthread_rwlock_unlock(&rwlock);
        // pthread_mutex_unlock(&mutex);

        // printf("Amount to transfer is: %d\n", transfer_amount);

        printf("Transfer thread: %ld removed %d from sender: %d and added to receiver: %d\n", my_rank, transfer_amount, sender, receiver);
        // printf("Sender balance is: %d\n", list[sender]);
        // printf("Receiver balance is: %d\n", list[receiver]);
    }
    free(argstruct);
    return NULL;
}

/*Thread function for performing reading/printing transactions*/
void *coarse_grained_read(void *arg) {

    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int reads = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    long balance_sum = 0;
    // Loop for transactions
    for (int i = 0; i < reads; i++) {
        // Pick a balance randomly to read its amount
        int bal = rand_from_range(balance_count);

        // pthread_mutex_lock(&mutex);
        // pthread_rwlock_rdlock(&rwlock);
        mtxrw_lock(lock_type, "reader", &mutex, &rwlock);
        balance_sum += list[bal];
        mtxrw_unlock(lock_type, &mutex, &rwlock);
        // pthread_rwlock_unlock(&rwlock);
        // pthread_mutex_unlock(&mutex);

        printf("Read balance: %d, of account: %d\n", list[bal], bal);
    }
    printf("Read thread: %ld, Sum of read balances is: %ld\n", my_rank, balance_sum);
    free(argstruct);

    return NULL;
}

void *fine_grained_transfer(void *arg) {
    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int transfers = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    // Loop for transactions
    for (int i = 0; i < transfers; i++) {
        // Pick a random amount to remove from sender and add to receiver
        int transfer_amount = rand_from_range(100);

        // Pick a balance randomly to remove a random amount from
        int sender = rand_from_range(balance_count);
        // printf("Sender is: %d, with balance: %d\n", sender, list[sender]);

        // pthread_mutex_lock(&mutex);
        mtxrw_lock(lock_type, "writer", &mutex_list[sender], &rw_list[sender]);
        list[sender] = list[sender] - transfer_amount;
        mtxrw_unlock(lock_type, &mutex_list[sender], &rw_list[sender]);
        // pthread_mutex_unlock(&mutex);

        // Pick another balance randomly to add the random amount to, make sure it's not the same as the first
        int receiver = rand_from_range(balance_count);
        while (receiver == sender) {
            receiver = rand_from_range(balance_count);
        }
        // printf("Receiver is: %d, with balance: %d\n", receiver, list[receiver]);

        // pthread_mutex_lock(&mutex);
        list[receiver] = list[receiver] + transfer_amount;
        // pthread_mutex_unlock(&mutex);
        // pthread_mutex_unlock(&mutex);

        // printf("Amount to transfer is: %d\n", transfer_amount);

        printf("Transfer thread: %ld removed %d from sender: %d and added to receiver: %d\n", my_rank, transfer_amount, sender, receiver);
        // printf("Sender balance is: %d\n", list[sender]);
        // printf("Receiver balance is: %d\n", list[receiver]);
    }
    free(argstruct);
    return NULL;
}

void *fine_grained_read(void *arg) {

    struct thread_arguments *argstruct = (struct thread_arguments *)arg;
    int *list = *argstruct->list_of_accounts;
    int reads = argstruct->number_of_transfers;
    long my_rank = argstruct->thread_rank;
    long balance_sum = 0;
    // Loop for transactions
    for (int i = 0; i < reads; i++) {
        // Pick a balance randomly to read its amount
        int bal = rand_from_range(balance_count);

        // pthread_mutex_lock(&mutex);
        mtxrw_lock(lock_type, "writer", &mutex_list[bal], &rw_list[bal]);
        balance_sum += list[bal];
        mtxrw_unlock(lock_type, &mutex_list[bal], &rw_list[bal]);
        // pthread_mutex_unlock(&mutex);

        printf("Read balance: %d, of account: %d\n", list[bal], bal);
    }
    printf("Read thread: %ld, Sum of read balances is: %ld\n", my_rank, balance_sum);
    free(argstruct);

    return NULL;
}

double time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// TODO: Implement functionality for different types of locks
// TODO: The number of transactions for each thread to perform is given as input, the total is calculated as (number of threads * number of transactions)
// a percentage of the total(given as input) is to be assigned to reading threads
int main(int argc, char *argv[]) {
    // Timespec initialization
    struct timespec parallel_execution_start, parallel_execution_finish;

    // Receiving program arguments
    balance_count = atoi(argv[1]);
    transactions_count = atoi(argv[2]);
    long read_percentage = strtol(argv[3], NULL, 10);
    lock_type = argv[4];

    // Checking and initializing lock type
    printf("Lock type: %s\n", lock_type);
    int res = mtxrw_init(lock_type, &mutex, &rwlock);
    if (res != 0) {
        printf("Result of init is: %d", res);
    }
    // pthread_mutex_init(&mutex, NULL);
    // pthread_rwlock_init(&rwlock, NULL);

    // Initializing list of locks
    mutex_list = malloc(balance_count * sizeof(pthread_mutex_t));
    rw_list = malloc(balance_count * sizeof(pthread_rwlock_t));
    for (int k = 0; k < balance_count; k++) {
        mtxrw_init(lock_type, &mutex_list[k], &rw_list[k]);
    }

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

    int total_transactions = transactions_count * thread_count;

    // Calculating number of reading transactions based on the total number of transactions and percentage of reading transactions
    // Using the ceil() function in cases where the calculation leads to a non-integer value, so that most transactions end up as reading ones
    int read_transactions_count = ceil(((read_percentage / (float)100) * total_transactions));
    printf("Number of reading transactions: %d\n", read_transactions_count);

    // Number of writing transactions is just however many transactions remain from the total
    int write_transactions_count = total_transactions - read_transactions_count;
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

    int transactions_per_read_thread = ceil(read_transactions_count / (double)reader_threads);
    printf("transactions_per_transfer_thread is: %d\n", transactions_per_read_thread);

    int remaining_transfers = write_transactions_count;
    int remaining_reads = read_transactions_count;

    timespec_get(&parallel_execution_start, TIME_UTC);

    // Initializing reader threads
    for (thread = 0; thread < reader_threads; thread++) {
        if (remaining_reads < transactions_per_read_thread) {
            transactions_per_read_thread = remaining_reads;
        }

        struct thread_arguments *args = (struct thread_arguments *)malloc(sizeof(struct thread_arguments));

        args->list_of_accounts = &balance_list;
        args->number_of_transfers = transactions_per_transfer_thread;
        args->thread_rank = thread;

        printf("Creating read thread: %ld\n", thread);
        pthread_create(&reader_handle[thread], NULL, fine_grained_read, (void *)args);

        remaining_reads -= transactions_per_read_thread;
    }

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

        printf("Creating transfer thread: %ld\n", thread);
        int create_res = pthread_create(&writer_handle[thread], NULL, fine_grained_transfer, (void *)args);
        if (create_res != 0) {
            printf("Thread creation error for thread %ld: %d(%s)\n", thread, create_res, strerror(create_res));
            break;
        }
        remaining_transfers -= transactions_per_transfer_thread;
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

    pthread_mutex_destroy(&mutex);
    pthread_rwlock_destroy(&rwlock);
    free(balance_list);

    return 0;
}
