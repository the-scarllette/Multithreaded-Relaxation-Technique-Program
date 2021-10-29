#include <malloc.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

void print_square(double *to_test, int size);
double average_value(double *array, int index, int size);
void average_square(double *array, double *new_array, int size);
void iterate(double *array1, double *array2, int size, double error_margin, bool print_iterations);
int get_array_index(int i, int j, int size);

void random_array(double *to_fill, int MAX_VALUE, int size){
    int num, den;
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            den = rand();
            den = den == 0? 1 : den;
            num = (rand() % (den*MAX_VALUE));
            to_fill[get_array_index(i, j, size)] = (double)num/(double)den;
        }
    }
}

void generate_tests(int num_tests, int MAX_VALUE, int size, double error_margin, bool print_iterations, bool print_start_end){
    double *to_test;
    double *new_array;
    for(int i = 0; i < num_tests; i++){
        to_test = malloc(size*size*sizeof(double));
        new_array = malloc(size*size*sizeof(double));

        random_array(to_test, MAX_VALUE, size);

        if(print_start_end) {
            printf("Test %d\n", i + 1);
            printf("Starting Array:\n");
            print_square(to_test, size);
            printf("\n");
        }

        iterate(to_test, new_array, size, error_margin, print_iterations);

        if(print_start_end) {
            printf("Iterated Array\n");
            print_square(new_array, size);
            printf("\n");
        }

        free(to_test);
        free(new_array);
    }
}

int main(){
    srand(time(NULL));
    int SIZE = 1000;
    int MAX_VALUE = 100;
    int num_test = 1;
    double error_margin = 0.001;

    generate_tests(num_test, MAX_VALUE, SIZE, error_margin, false, false);

    return 0;
}