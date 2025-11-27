#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

long N;
long THREAD_COUNT;
pthread_mutex_t mutex;
pthread_cond_t cond;
long waiting_thread_count = 0; // used in barrier implementation

// checks command usage; returns 0 on success, -1 otherwise
int parse_args(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <N iterations> <thread count>\n", argv[0]);
        return -1;
    }
        
    char *end;
    N = strtol(argv[1], &end, 10);
    if (*end != '\0' || N <= 0)
    {
        fprintf(stderr, "Invalid N.\n");
        return -1;
    }

    THREAD_COUNT = strtol(argv[2], &end, 10);
    if (*end != '\0' || THREAD_COUNT <= 0)
    {
        fprintf(stderr, "Invalid thread count.\n");
        return -1;
    }

    return 0;
}

double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void *loop_worker(void *args)
{
    for (long i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);
        waiting_thread_count++;
        if (waiting_thread_count == THREAD_COUNT) // this is the last thread to reach the barrier
        {
            waiting_thread_count = 0; // to ensure reusability for the barrier
            pthread_cond_broadcast(&cond); // waking all threads to continue
        }
        else // this is not the last thread
        {
            // must wait for the remaining threads to reach here
            while (pthread_cond_wait(&cond, &mutex) != 0);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    if (parse_args(argc, argv) == -1) return 1;

    pthread_t *thread_handles = malloc(THREAD_COUNT * sizeof(pthread_t));
    if (!thread_handles) return 1;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

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

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    free(thread_handles);
    return 0;
}