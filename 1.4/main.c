#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

long thread_count;

void *foo(void *arg) {
    printf("Foo works\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    thread_count = atol(argv[1]);
    pthread_t *thread_handle = malloc(thread_count * sizeof(pthread_t));

    // Initializing list with 5 balance accounts
    int *balance_list = malloc(5 * sizeof(int));

    // Adding random values to list
    printf("List should be: \n");
    printf("[");
    for (int i = 0; i < 5; i++) {
        balance_list[i] = rand();
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n");

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handle[thread], NULL, foo, (void *)&balance_list);
    }

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handle[thread], NULL);
    }

    return 0;
}
