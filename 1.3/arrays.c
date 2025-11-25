#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdalign.h> // needed for using alignas()

#if BETTER == 1 // BETTER is defined in the compilation command

// aligns both the whole struct and each of its members to 64 bytes, using appropriate padding
// assuming a cache line is 64 bytes, this avoids false sharing

#define CACHELINE 64
alignas(CACHELINE) struct array_stats_s {
    alignas(CACHELINE) long long int info_array_0;
    alignas(CACHELINE) long long int info_array_1;
    alignas(CACHELINE) long long int info_array_2;
    alignas(CACHELINE) long long int info_array_3;
} array_stats;

#else

struct array_stats_s {
    long long int info_array_0;
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
} array_stats;

#endif

long long N;
int *arrays[4];

// checks command usage; returns 0 on success, -1 otherwise
int parse_args(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <array length>\n", argv[0]);
        return -1;
    }
        
    char *end;
    N = strtoll(argv[1], &end, 10);
    if (*end != '\0' || N <= 0)
    {
        fprintf(stderr, "Invalid array length.\n");
        return -1;
    }

    return 0;
}

double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// allocates the 4 arrays with N integers, consistently freeing on errors
// returns 0 on success, -1 otherwise
int allocate_arrays(void)
{
    for (int i = 0; i < 4; i++)
    {
        arrays[i] = malloc(N * sizeof(int));
        if (!arrays[i])
        {
            for (int j = i - 1; j >= 0; j--)
                free(arrays[j]);

            return -1;
        }
    }

    return 0;
}

void free_arrays(void)
{
    for (int i = 0; i < 4; i++)
        free(arrays[i]);
}

// assigns random values 0-9 to an array of length N
// srand() should be called before calling this
void assign_random_values(int *array)
{
    for (long long i = 0; i < N; i++)
        array[i] = rand() % 10;
}

// returns 1 if equals, 0 otherwise
int array_stats_equals(struct array_stats_s *stats1, struct array_stats_s *stats2)
{
    if (stats1->info_array_0 != stats2->info_array_0) return 0;
    if (stats1->info_array_1 != stats2->info_array_1) return 0;
    if (stats1->info_array_2 != stats2->info_array_2) return 0;
    if (stats1->info_array_3 != stats2->info_array_3) return 0;

    return 1;
}

// prints array_stats (global variable), useful for test purposes
void print_array_stats(void)
{
    printf("%lld, ", array_stats.info_array_0);
    printf("%lld, ", array_stats.info_array_1);
    printf("%lld, ", array_stats.info_array_2);
    printf("%lld\n", array_stats.info_array_3);
}

// counts non-zero elements of all arrays and updates array_stats serially
void count_non_zero_serial(void)
{
    long long *info_array[4];
    info_array[0] = &(array_stats.info_array_0);
    info_array[1] = &(array_stats.info_array_1);
    info_array[2] = &(array_stats.info_array_2);
    info_array[3] = &(array_stats.info_array_3);

    for (int i = 0; i < 4; i++)
    {
        *info_array[i] = 0; // initializing to 0
        for (long long j = 0; j < N; j++)
            if (arrays[i][j] != 0)
                (*info_array[i])++;
    }
}

// counts non-zero elements of the corresponding array to rank and updates the related info only 
void *count_non_zero_worker(void *rank)
{
    long my_rank = (long)rank;

    // determining in which info to write
    long long *selected_info;
    switch (my_rank)
    {
        case 0:
            selected_info = &(array_stats.info_array_0);
            break;
        case 1:
            selected_info = &(array_stats.info_array_1);
            break;
        case 2:
            selected_info = &(array_stats.info_array_2);
            break;
        case 3:
            selected_info = &(array_stats.info_array_3);
            break;
        default:
            return NULL; // to avoid gcc warnings
    }

    *selected_info = 0; // initializing to 0
    for (long long i = 0; i < N; i++)
        if (arrays[my_rank][i] != 0)
            (*selected_info)++;

    return NULL;
}

int main(int argc, char *argv[])
{
    #if BETTER == 1
    printf("running with BETTER\n");
    #endif

    struct timespec start_init, end_init, start_serial, end_serial, start_parallel, end_parallel;
    timespec_get(&start_init, TIME_UTC);

    if (parse_args(argc, argv) == -1) return 1;

    srand(time(NULL));

    // allocating and initializing the arrays
    if (allocate_arrays() == -1) return 1;

    for (int i = 0; i < 4; i++)
        assign_random_values(arrays[i]);

    timespec_get(&end_init, TIME_UTC);
    printf("Initialization:     %.6f sec\n", elapsed(start_init, end_init));

    timespec_get(&start_serial, TIME_UTC);

    // running the serial algorithm
    count_non_zero_serial();

    timespec_get(&end_serial, TIME_UTC);
    printf("Serial algorithm:   %.6f sec\n", elapsed(start_serial, end_serial));

    // saving previous result to compare later
    struct array_stats_s prev_result_copy;
    memcpy(&prev_result_copy, &array_stats, sizeof(struct array_stats_s));

    // creating threads
    pthread_t *thread_handles = malloc(4 * sizeof(pthread_t));
    if (!thread_handles)
    {
        free_arrays();
        return 1;
    }

    timespec_get(&start_parallel, TIME_UTC);

    for (long thread = 0; thread < 4; thread++)
        pthread_create(&thread_handles[thread], NULL, count_non_zero_worker, (void *)thread);

    // waiting for the threads to finish
    for (long thread = 0; thread < 4; thread++)
        pthread_join(thread_handles[thread], NULL);

    timespec_get(&end_parallel, TIME_UTC);
    printf("Parallel algorithm: %.6f sec\n", elapsed(start_parallel, end_parallel));

    // comparing the previous result with the updated array_stats
    if (array_stats_equals(&prev_result_copy, &array_stats))
        printf("Consistent results\n");
    else
        printf("Inconsistent results\n");

    //printf("Speed up: %.3f\n", elapsed(start_serial, end_serial) / elapsed(start_parallel, end_parallel));

    free_arrays();

    return 0;
}

