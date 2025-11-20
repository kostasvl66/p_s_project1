#include <stdio.h>
#include <stdlib.h>

int main() {
    // Initializing list with 5 balance accounts
    int *balance_list = malloc(5 * sizeof(int));

    printf("List should be: \n");
    printf("[");
    for (int i = 0; i < 5; i++) {
        balance_list[i] = rand();
        printf("\t%d,", balance_list[i]);
    }
    printf("\t]\n");

    return 0;
}
