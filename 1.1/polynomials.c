#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// each polynomial coefficient a non-zero int within [-MAX_ABS_COEFFICIENT_VALUE, +MAX_ABS_COEFFICIENT_VALUE]
#define MAX_ABS_COEFFICIENT_VALUE 1000

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

// calculates time difference between start and end with nsec precision
double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

typedef struct {
    int *coef_arr; // array of coefficients, each index maps to the same power
    long degree; // the largest power
} Polynomial;

// returns a random non-zero coefficient array of length (degree + 1) or NULL if the allocation fails
// the caller receives ownership of the allocated memory
// srand() must be called before the function call
int *generate_random_coef(long degree)
{
    int *coef_arr = malloc((degree + 1) * sizeof(int));
    if (!coef_arr) return NULL;

    for (long i = 0; i <= degree; i++)
    {
        // making sure each coefficient is non-zero
        int abs_val = 1 + rand() % MAX_ABS_COEFFICIENT_VALUE; // 1 <= abs_val <= MAX_ABS_COEFFICIENT_VALUE
        int sign = (rand() % 2) ? 1 : -1; // random sign
        coef_arr[i] = sign * abs_val; // 1 <= abs(coef_arr[i]) <= MAX_ABS_COEFFICIENT_VALUE
    }

    return coef_arr;
}

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

// returns 1 if pol1 and pol2 have the same degree and coefficients; 0 otherwise
int pol_equals(Polynomial *pol1, Polynomial *pol2)
{
    if (pol1->degree != pol2->degree) return 0;
    
    for (long i = 0; i <= pol1->degree; i++)
        if (pol1->coef_arr[i] != pol2->coef_arr[i])
            return 0;

    return 1;
}

// sets every term of a polynomial to zero without changing degree (useful for multiplication)
void pol_set_zero(Polynomial *pol)
{
    for (long i = 0; i <= pol->degree; i++)
        pol->coef_arr[i] = 0;
}

// result receives the sum of two polynomials; returns 0 if successful, -1 otherwise
// result is expected to have allocated large enough coef_arr, and only the terms up to the degree are updated
// this function works properly even if pol1 or pol2 points to the same memory as result
void pol_add(Polynomial *pol1, Polynomial *pol2, Polynomial *result)
{
    // saving degrees in case pol1 or pol2 points to the same memory as result
    long degree1 = pol1->degree;
    long degree2 = pol2->degree;

    // new degree is the largest of the two
    result->degree = (degree1 > degree2) ? degree1 : degree2;

    long min_degree = (degree1 < degree2) ? degree1 : degree2; // smallest of the two degrees
    for (long i = 0; i <= min_degree; i++)
        result->coef_arr[i] = pol1->coef_arr[i] + pol2->coef_arr[i];

    Polynomial *max_pol = (degree1 > degree2) ? pol1 : pol2; // polynomial with the largest degree
    for (long i = min_degree + 1; i <= result->degree; i++)
        result->coef_arr[i] = max_pol->coef_arr[i];
}

// returns the product of two polynomials; its memory is to be freed using pol_destroy; returns NULL if unsuccessful
Polynomial *pol_multiply(Polynomial *pol1, Polynomial *pol2)
{
    Polynomial *res = malloc(sizeof(Polynomial));
    if (!res) return NULL;

    // new degree is the sum of the two
    res->degree = pol1->degree + pol2->degree;
    res->coef_arr = malloc((res->degree + 1) * sizeof(int));
    if (!(res->coef_arr))
    {
        free(res);
        return NULL;
    }

    Polynomial prod_i; // the product of i-th pol1 term and the whole pol2
    prod_i.degree = 0; // for now
    prod_i.coef_arr = malloc((res->degree + 1) * sizeof(int)); // pre-allocating the final needed size to avoid allocations in the loop
    if (!(prod_i.coef_arr))
    {
        free(res->coef_arr);
        free(res);
        return NULL;
    }

    for (long i = 0; i <= pol1->degree; i++)
    {
        prod_i.degree = pol2->degree + i;
        pol_set_zero(&prod_i); // setting zero to overrite terms of power lower than (j + i)
        for (long j = 0; j <= pol2->degree; j++)
            prod_i.coef_arr[j + i] = pol1->coef_arr[i] * pol2->coef_arr[j]; // "shifting" each power by i and multiplying terms

        pol_add(&prod_i, res, res); // adding prod_i to res
    }

    free(prod_i.coef_arr);
    return res;
}

int main(int argc, char *argv[])
{   
    if (parse_args(argc, argv) == -1) return 1;

    srand(time(NULL));

    int *coef_arr1 = generate_random_coef(N);
    if (!coef_arr1) return 1;
    Polynomial *pol1;
    if (pol_init(&pol1, coef_arr1, N) == -1) return 1;
    pol_print(pol1);

    int *coef_arr2 = generate_random_coef(THREAD_COUNT);
    if (!coef_arr2) return 1;
    Polynomial *pol2;
    if (pol_init(&pol2, coef_arr2, THREAD_COUNT) == -1) return 1;
    pol_print(pol2);

    struct timespec start, end;

    timespec_get(&start, TIME_UTC);

    Polynomial *prod = pol_multiply(pol1, pol2);
    if (!prod) return 1;
    
    timespec_get(&end, TIME_UTC);
    printf("Elapsed time: %.9f sec\n", elapsed(start, end));
    pol_print(prod);

    pol_destroy(&pol1);
    pol_destroy(&pol2);
    pol_destroy(&prod);

    return 0;
}