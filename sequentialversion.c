#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <math.h>

struct doublesArraySection{
    int size;
    double *array;
    double *newarray;
} doublesArraySection;

int rand(void);
void srand(unsigned int seed);

int get_array_index(int i, int j, int size){
    return size*i + j;
}

double average_value(double *array, int index, int size){
    return (array[index - 1] + array[index + 1] + array[index - size] + array[index + size])/4.0;
}

void* average_square(void *sectionaddress){
    struct doublesArraySection *arraysection = sectionaddress;
    int index;
    for(int i = 1; i < (*arraysection).size - 1; i++){
        for(int j = 1; j < (*arraysection).size - 1; j++){
            index = get_array_index(i, j, (*arraysection).size);
            (*arraysection).newarray[index] = average_value((*arraysection).array, index, (*arraysection).size);
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
            printf("%.3f, ", to_print[get_array_index(i, j, size)]);
        }
        printf("\n");
    }
}

bool within_error(double *array, double *new_array, double error_margin, int size){
    printf("Checking if Within Error");
    int index;
    double difference;
    for(int i = 1; i < size - 1; i++){
        for(int j = 1; j < size - 1; j++){
            index = get_array_index(i, j, size);
            difference = fabs(array[index] - new_array[index]);
            if(difference >= error_margin){
                return false;
            }
        }
    }

    return true;
}

void iterate(double *array1, double *array2, int size, double error_margin, bool print_iterations){
    copy_boundary(array1, array2, size);

    bool keep_iterating = true;
    int k = 0;
    struct doublesArraySection arraySection;
    arraySection.size = size;
    while(keep_iterating){
        arraySection.array = array1;
        arraySection.newarray = array2;
        average_square(&arraySection);
        k++;
        if(within_error(array1, array2, error_margin, size)){
            printf("%d iterations\n", k);
            return;
        }

        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array2, size);
        }

        arraySection.array = array2;
        arraySection.newarray = array1;
        average_square(&arraySection);
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

void load_test_cases(double *to_test, int size, int iteration){
    double to_store = (double)(iteration + 1);
    for(int i = 0; i < size; i++){
        to_test[get_array_index(i, 0, size)] = to_store;
        to_test[get_array_index(i, size - 1, size)] = to_store;
    }

    for(int j = 0; j < size; j++){
        to_test[get_array_index(0, j, size)] = to_store;
        to_test[get_array_index(size - 1, j, size)] = to_store;
    }

    for(int i = 1; i < size - 1; i++){
        for(int j = 1; j < size - 1; j++){
            to_test[get_array_index(i, j, size)] = 0.0;
        }
    }
}

void generate_tests(int num_tests, int MAX_VALUE, int size, double error_margin,
                    bool use_random_array, bool print_iterations, bool print_start_end){
    double *to_test, *new_array;
    long int start_time, end_time, run_time;
    for(int i = 0; i < num_tests; i++){
        to_test = malloc(size*size*sizeof(double));
        new_array = malloc(size*size*sizeof(double));

        if(use_random_array) {
            random_array(to_test, MAX_VALUE, size);
        }else{
            load_test_cases(to_test, size, i);
        }

        if(print_start_end) {
            printf("Test %d\n", i + 1);
            printf("Starting Array:\n");
            print_square(to_test, size);
            printf("\n");
        }

        start_time = (long int)(time(NULL));
        iterate(to_test, new_array, size, error_margin, print_iterations);
        end_time = (long int)(time(NULL));
        run_time = end_time - start_time;
        printf("Took %d seconds to run\n", run_time);

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
    double error_margin = 0.1;
    bool print_iterations = true;
    bool print_start_end = true;

    bool use_random_array = false;

    generate_tests(num_test, MAX_VALUE, SIZE, error_margin,
                   use_random_array, print_iterations, print_start_end);

    return 0;
}
