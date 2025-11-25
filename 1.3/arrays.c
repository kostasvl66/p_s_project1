#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct array_stats_s {
    long long int info_array_0;
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
} array_stats;

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

// assigns random values 0-9 to an array of length N
// srand() should be called before calling this
void assign_random_values(int *array)
{
    for (long long i = 0; i < N; i++)
        array[i] = rand() % 10;
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
            if (info_array[i][j] != 0)
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
    }

    *selected_info = 0; // initializing to 0
    for (long long i = 0; i < N; i++)
        if (arrays[my_rank][i] != 0)
            (*selected_info)++;

    return NULL;
}

int main(int argc, char *argv[])
{
    if (parse_args(argc, argv) == -1) return 1;

    srand(time(NULL));

    // allocating and initializing the arrays
    if (allocate_arrays() == -1) return 1;

    for (int i = 0; i < 4; i++)
        assign_random_values(arrays[i]);

    // creating threads
    pthread_t thread_handles[4];
    for (long thread = 0; thread < 4; thread++)
        pthread_create(&thread_handles[thread], NULL, count_non_zero_worker, (void *)thread);

    // waiting for the threads to finish
    for (long thread = 0; thread < 4; thread++)
        pthread_join(thread_handles[thread], NULL);

    // saving previous result to compare later
    struct array_stats_s threaded_result_copy;
    memcpy(&threaded_result_copy, &array_stats, sizeof(struct array_stats_s));

    // trying the serial algorithm
    count_non_zero_serial();

    // comparing the previous result with the updated array_stats
    if (memcmp(&threaded_result_copy, &array_stats, sizeof(struct array_stats_s)) == 0)
        printf("Consistent results\n");
    else
        printf("Inconsistent results\n");

    return 0;
}

