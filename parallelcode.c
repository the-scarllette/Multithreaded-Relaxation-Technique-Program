#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>

struct doublesArraySection{
    int size;
    double *array;
    double *newarray;
    int row_start;
    int num_rows;
} doublesArraySection;

int rand(void);
void srand(unsigned int seed);

int get_array_index(int i, int j, int size){
    return size*i + j;
}

double average_value(double *array, int index, int size){
    return (array[index - 1] + array[index + 1] + array[index - size] + array[index + size])/4.0;
}

void* average_section(void *sectionaddress){
    struct doublesArraySection *arraySection = sectionaddress;
    int index;
    for(int i = (*arraySection).row_start; i < (*arraySection).row_start + (*arraySection).num_rows; i++){
        for(int j = 1; j < (*arraySection).size; j++){
            index = get_array_index(i, j, (*arraySection).size);
            (*arraySection).newarray[index] = average_value((*arraySection).array, index, (*arraySection).size);
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

void partition_array(struct doublesArraySection *sections, int num_sections, int array_size){
    int rows_per_thread = (array_size - 2)/num_sections;
    for(int k = 0; k < num_sections; k++){
        sections[k].size = array_size;
        sections[k].row_start = k*rows_per_thread + 1;
        sections[k].num_rows = rows_per_thread;
    }
    sections[num_sections - 1].num_rows = (array_size - 1) - sections[num_sections - 1].row_start;
    for(int k = 0; k < num_sections; k++){
        printf("Section %d starts on row %d and covers %d rows\n", k, sections[k].row_start, sections[k].num_rows);
    }
}

void iterate(int num_threds, double *array1, double *array2, int size, double error_margin, bool print_iterations){
    copy_boundary(array1, array2, size);

    bool keep_iterating = true;
    int k = 0;
    struct doublesArraySection *sections = malloc(num_threds*sizeof(doublesArraySection));
    partition_array(sections, num_threds, size);
    while(keep_iterating){
        if(k % 500 == 0){
            printf("iteration: %d\n", k);
        }

        for(int i = 0; i < num_threds; i++){
            sections[i].array = array1;
            sections[i].newarray = array2;
            average_section(&sections[i]);
        }
        k++;
        if(within_error(array1, array2, error_margin, size)){
            printf("%d iterations\n", k);
            free(sections);
            return;
        }

        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array2, size);
        }

        for(int i = 0; i < num_threds; i++){
            sections[i].array = array2;
            sections[i].newarray = array1;
            average_section(&sections[i]);
        }
        k++;
        if(within_error(array2, array1, error_margin, size)){
            printf("%d iterations\n", k);
            free(sections);
            return;
        }

        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array1, size);
        }
    }
    free(sections);
}


void random_array(double *to_fill, int MAX_VALUE, int size){
    int num, den;
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            den = rand();
            den = den == 0? 1 : den;
            num = (rand() % (MAX_VALUE));
            to_fill[get_array_index(i, j, size)] = (double)num;
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
    int SIZE = 4;
    int MAX_VALUE = 100;
    int num_test = 1;
    int num_threads = 3;
    double error_margin = 0.1;
    bool print_iterations = false;
    bool print_start_end = true;

    generate_tests(num_threads, num_test, MAX_VALUE, SIZE, error_margin, print_iterations, print_start_end);

    return 0;
}
