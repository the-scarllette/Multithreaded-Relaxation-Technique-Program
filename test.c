#include <malloc.h>
#include <time.h>
#include <stdio.h>

void print_square(double *to_test, int size);
double average_value(double *array, int index, int size);
void average_square(double *array, double *new_array, int size);
void copy_boundary(double *array, double *new_array, int size);

int main(){
    srand(time(NULL));
    int size = 4;
    int k = 0;

    double *to_test = malloc(size*size*sizeof(double));
    double *new_array = malloc(size*size*sizeof(double));

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            to_test[size*i + j] = (double)(rand() % 10);
        }
    }

    printf("Starting Array:\n");
    print_square(to_test, size);
    printf("\n");

    copy_boundary(to_test, new_array, size);
    average_square(to_test, new_array, size);

    printf("Iterated Array\n");
    print_square(new_array, size);
    printf("\n");

    free(to_test);
    free(new_array);
    return 0;
};