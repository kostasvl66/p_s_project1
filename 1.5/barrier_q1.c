#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long N;
long THREAD_COUNT;

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

int main(int argc, char *argv[])
{
    if (parse_args(argc, argv) == -1) return 1;

    

    return 0;
}

