#include <stdio.h>
#include <stdbool.h>

int get_array_index(int i, int j, int size){
    return size*i + j;
}

double average_value(double *array, int index, int size){
    return (array[index - 1] + array[index + 1] + array[index - size] + array[index + size])/4.0;
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
    for(int i = 1; i < size - 1; i++){
        for(int j = 1; j < size - 1; j++){
            index = get_array_index(i, j, size);
            if((array[index]*array[index] - new_array[index]*new_array[index]) >= error_margin){
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
    while(keep_iterating){
        if(k % 500 == 0){
            printf("iteration: %d\n", k);
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
