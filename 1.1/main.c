#include <pthread.h>
#include <stdio.h>

#define MAX_THREADS 4

/*Simple function to iteratively increment a shared integer 5 times*/
void increment(int *num) {
    for (int i = 0; i < 5; i++) {
        *num += 1;
    }
}

int main() {
    printf("------------Starting main-------------\n");

    int shared = 0;
    printf("Starting value of shared variable is: %d\n", shared);
    int *ptr = &shared;

    for (int i = 0; i < MAX_THREADS; i++) {
        increment(ptr);
    }

    // Expected final value is 20, since we have 4 threads incrementing the value 5 times each
    printf("Final value of variable is: %d\n", shared);
    printf("----------Shutting down main----------\n");

    return 0;
}
