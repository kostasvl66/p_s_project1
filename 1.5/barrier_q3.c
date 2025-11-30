#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long N;
long THREAD_COUNT;

struct srcBarrier {
    int max_barred_threads;
    int total_threads_barred;
    pthread_mutex_t lock;
    bool sense_flag;
};

struct srcBarrier barrier;

/*Function for initializing the barrier structure*/
void srcBarrier_init(struct srcBarrier *barrier, pthread_mutexattr_t *mutex_attr, int barrier_max) {
    pthread_mutex_init(&(barrier->lock), mutex_attr);
    barrier->max_barred_threads = barrier_max;
    barrier->total_threads_barred = 0;
    barrier->sense_flag = false;
}

/*Function for setting the barrier to a waiting state*/
void srcBarrier_wait(struct srcBarrier *barrier) {
    bool private_sense = barrier->sense_flag;

    if (!pthread_mutex_lock(&(barrier->lock))) {
        barrier->total_threads_barred++;
        private_sense = !private_sense;

        if (barrier->total_threads_barred == barrier->max_barred_threads) {
            barrier->total_threads_barred = 0;
            barrier->sense_flag = private_sense;
            pthread_mutex_unlock(&(barrier->lock));
        } else {
            pthread_mutex_unlock(&(barrier->lock));
            while (barrier->sense_flag != private_sense)
                ;
        }
    }
}

void srcBarrier_destroy(struct srcBarrier *barrier) {
    pthread_mutex_destroy(&(barrier->lock));
}

// checks command usage; returns 0 on success, -1 otherwise
int parse_args(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N iterations> <thread count>\n", argv[0]);
        return -1;
    }

    char *end;
    N = strtol(argv[1], &end, 10);
    if (*end != '\0' || N <= 0) {
        fprintf(stderr, "Invalid N.\n");
        return -1;
    }

    THREAD_COUNT = strtol(argv[2], &end, 10);
    if (*end != '\0' || THREAD_COUNT <= 0) {
        fprintf(stderr, "Invalid thread count.\n");
        return -1;
    }

    return 0;
}

double elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void *loop_worker(void *args) {
    // long my_rank = *((int *)args);

    for (long i = 0; i < N; i++) {
        printf("thread id %ld is waiting at the barrier, as not enough %d threads are running ...\n", pthread_self(), barrier.max_barred_threads);
        srcBarrier_wait(&barrier);
        printf("The barrier is lifted, thread id %ld is running now\n", pthread_self());
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (parse_args(argc, argv) == -1)
        return 1;

    pthread_t *thread_handles = malloc(THREAD_COUNT * sizeof(pthread_t));
    if (!thread_handles)
        return 1;

    // Initializing barrier with a maximum barrier limit of 3 threads
    srcBarrier_init(&barrier, NULL, 4);

    struct timespec start, end;
    timespec_get(&start, TIME_UTC);

    // creating threads
    for (long thread = 0; thread < THREAD_COUNT; thread++)
        pthread_create(&thread_handles[thread], NULL, loop_worker, NULL);

    // waiting for threads to finish
    for (long thread = 0; thread < THREAD_COUNT; thread++)
        pthread_join(thread_handles[thread], NULL);

    timespec_get(&end, TIME_UTC);
    printf("Time: %.6f sec\n", elapsed(start, end));

    srcBarrier_destroy(&barrier);
    free(thread_handles);
    return 0;
}
