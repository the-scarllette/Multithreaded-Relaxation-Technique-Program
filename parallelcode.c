#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <pthread.h>
/*
 * create struct
 * pass pointer to thread
 * extract struct values
 * close threads
 */

struct

int rand(void);
void srand(unsigned int seed);

int get_array_index(int i, int j, int size){
    return size*i + j;
}

double average_value(double *array, int index, int size){
    return (array[index - 1] + array[index + 1] + array[index - size] + array[index + size])/4.0;
}

void average_section(double, *array, double *new_array, int size, int start, int end){
for{int i = start, i <= end, i++}{
new_array[i] = average_value(array, i, size);
}
}

void average_square(double *array, double *new_array, int size){
    int index;
    for(int i = 1; i < size - 1; i++){
        for(int j = 1;j < size - 1; j++){
            index = get_array_index(i, j, size);
            new_array[index] = average_value(array, index, size);
        }
    }
}

void copy_boundary(double *array, double *new_array, int size){
    //copying top edge
    for(int k = 0; k < size; k++){
        new_array[k] = array[k];
    }

    //copying bottom edge
    int index;
    for(int k = 0; k < size; k++){
        index = size*(size - 1) + k;
        new_array[index] = array[index];
    }

    //copying right edge
    for(int k = 0; k < size; k++){
        index = size*k;
        new_array[index] = array[index];
    }

    //copying left edge
    for(int k = 0; k < size; k++){
        index = (k + 1)*size - 1;
        new_array[index] = array[index];
    }
}

void print_square(double *to_print, int size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            printf("%.3f, ", to_print[size*i + j]);
        }
        printf("\n");
    }
}

bool within_error(double *array, double *new_array, double error_margin, int size){
    int index;
    double difference;
    for(int i = 1; i < size - 1; i++){
        for(int j = 1; j < size - 1; j++){
            index = get_array_index(i, j, size);
            difference = (array[index]*array[index] - new_array[index]*new_array[index]);
            if(difference*difference >= error_margin*error_margin){
                return false;
            }
        }
    }

    return true;
}

void iterate(int num_threads, double *array1, double *array2, int size, double error_margin, bool print_iterations){
    copy_boundary(array1, array2, size);

    bool keep_iterating = true;
    int k = 0;

    //Creating Threads
    pthread_t *threads = malloc(num_threads*sizeof(pthread_t));

    while(keep_iterating){
        //Creating Threads
        for{int i = 0; i < num_threads; i++}{
            pthread_create(&threads[i], NULL, average_section, );
        }

        average_square(array1, array2, size);
        k++;
        if(within_error(array1, array2, error_margin, size)){
            printf("%d iterations\n", k);
            return;
        }

        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array2, size);
        }

        average_square(array2, array1, size);
        k++;
        if(within_error(array2, array1, error_margin, size)){
            printf("%d iterations\n", k);
            return;
        }

        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array1, size);
        }
    }
}


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

void generate_tests(int num_threads, int num_tests, int MAX_VALUE, int size, double error_margin, bool print_iterations, bool print_start_end){
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

        iterate(num_threads, to_test, new_array, size, error_margin, print_iterations);

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
    int SIZE = 10;
    int MAX_VALUE = 100;
    int num_test = 1;
    int num_threads = 2;
    double error_margin = 0.001;
    bool print_start_end = true;
    bool print_iterations = false;

    printf("lets do this\n");

    generate_tests(num_threads, num_test, MAX_VALUE, SIZE, error_margin, print_iterations, print_start_end);

    return 0;
}
