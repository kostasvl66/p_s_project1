#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fileptr;

    int sample = atoi(argv[1]);
    printf("Number of sample executions for each program is: %d\n", sample);

    char data[sample * 4][10];
    double result_list[4];

    fileptr = fopen("results.txt", "r");

    int i = 0;
    while (!feof(fileptr) && i < sample * 4) {
        fgets(data[i], 10, fileptr);
        i++;
    }

    fclose(fileptr);

    double sample_sum = 0.0;
    for (int j = 0; j < sample * 4; j++) {
        if (j % 4 == 0) {
            if (j != 0) {
                result_list[j / 4 - 1] = sample_sum / (double)sample;
            }
            sample_sum = strtod(data[j], NULL);
            continue;
        }
        sample_sum += strtod(data[j], NULL);
    }

    printf("Average execution time of main(non-deterministic) implementation is: %lf\n", result_list[0]);
    printf("Average execution time of mutex implementation is: %lf\n", result_list[1]);
    printf("Average execution time of atomic variable implementation is: %lf\n", result_list[2]);
    printf("Average execution time of readers-writer implementation is: %lf\n", result_list[3]);

    return 0;
}
