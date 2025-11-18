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
// coef_arr must be heap allocated and is now owned by pol_init; that array is to be freed by pol_destroy (or pol_init)
int pol_init(Polynomial **out_pol, int *coef_arr, long degree)
{
    *out_pol = malloc(sizeof(Polynomial));
    if (!(*out_pol))
    {
        free(coef_arr); // if malloc fails coef_arr must be freed as it is now owned by pol_init
        return -1;
    } 

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

// returns the sum of two polynomials; its memory is to be freed using pol_destroy; returns NULL if unsuccessful
Polynomial *pol_add(Polynomial *pol1, Polynomial *pol2)
{
    Polynomial *res = malloc(sizeof(Polynomial));
    if (!res) return NULL;

    // new degree is the largest of the two
    res->degree = (pol1->degree > pol2->degree) ? pol1->degree : pol2->degree;
    res->coef_arr = malloc((res->degree + 1) * sizeof(int));
    if (!(res->coef_arr))
    {
        free(res);
        return NULL;
    } 

    long min_degree = (pol1->degree < pol2->degree) ? pol1->degree : pol2->degree; // smallest of the two degrees
    for (int i = 0; i <= min_degree; i++)
        res->coef_arr[i] = pol1->coef_arr[i] + pol2->coef_arr[i];

    Polynomial *max_pol = (pol1->degree > pol2->degree) ? pol1 : pol2; // polynomial with the largest degree
    for (int i = min_degree + 1; i <= res->degree; i++)
        res->coef_arr[i] = max_pol->coef_arr[i];

    return res;
}

int main(int argc, char *argv[])
{   
    if (parse_args(argc, argv) == -1) return 1;

    // for now just trying the sum

    int *coef_arr1 = malloc((N + 1) * sizeof(int));
    if (!coef_arr1) return 1;
    for (int i = 0; i <= N; i++)
        coef_arr1[i] = 5;
    Polynomial *pol1;
    if (pol_init(&pol1, coef_arr1, N) == -1) return 1;
    pol_print(pol1);

    int *coef_arr2 = malloc((THREAD_COUNT + 1) * sizeof(int));
    if (!coef_arr2) return 1;
    for (int i = 0; i <= THREAD_COUNT; i++)
        coef_arr2[i] = 3;
    Polynomial *pol2;
    if (pol_init(&pol2, coef_arr2, THREAD_COUNT) == -1) return 1;
    pol_print(pol2);

    Polynomial *sum = pol_add(pol1, pol2);
    if (!sum) return 1;
    pol_print(sum);

    pol_destroy(&pol1);
    pol_destroy(&pol2);

    return 0;
}