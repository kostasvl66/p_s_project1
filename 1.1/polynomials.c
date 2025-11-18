#include <stdio.h>
#include <stdlib.h>

long N;
long THREAD_COUNT;

// checks command usage; returns 0 on success, -1 otherwise
int parse_args(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <pol degree> <thread count>\n", argv[0]);
        return -1;
    }
        
    char *end;
    N = strtol(argv[1], &end, 10);
    if (*end != '\0' || N < 0)
    {
        fprintf(stderr, "Invalid polynomial degree.\n");
        return -1;
    }
        
    THREAD_COUNT = strtol(argv[2], &end, 10);
    if (*end != '\0' || THREAD_COUNT < 1)
    {
        fprintf(stderr, "Invalid thread count.\n");
        return -1;
    }

    return 0;
}

typedef struct {
    int *coef_arr; // array of coefficients, each index maps to the same power
    long degree; // the largest power
} Polynomial;

// initializes a polynomial; returns 0 on success, -1 otherwise
// coef_arr must be heap allocated and is now owned by pol_init; that array is to be freed only by pol_destroy
int pol_init(Polynomial **out_pol, int *coef_arr, long degree)
{
    *out_pol = malloc(sizeof(Polynomial));
    if (!(*out_pol)) return -1;

    (*out_pol)->coef_arr = coef_arr; // assuming the coef_arr passed remains intact
    (*out_pol)->degree = degree;
    return 0;
}

// deallocates the polynomial's data
void pol_destroy(Polynomial **pol)
{
    free((*pol)->coef_arr);
    free(*pol);
    *pol = NULL;
}

// prints a polynomial for test purposes
void pol_print(Polynomial *pol)
{
    for (long i = pol->degree; i >= 0; i--)
    {
        printf("(%d)x^%ld", pol->coef_arr[i], i);
        if (i > 0)
            printf("+");
        else
            printf("\n");
    }
}

int main(int argc, char *argv[])
{   
    if (parse_args(argc, argv) == -1) return 1;

    int *coef_arr = malloc((N + 1) * sizeof(int));
    if (!coef_arr) return 1;
    for (int i = 0; i <= N; i++)
        coef_arr[i] = 5;

    Polynomial *pol;
    if (pol_init(&pol, coef_arr, N) == -1) return 1;

    pol_print(pol);

    pol_destroy(&pol);

    return 0;
}