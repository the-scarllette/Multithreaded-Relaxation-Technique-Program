
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <pthread.h>

pthread_barrier_t barrier1, barrier2;

struct doublesArraySection{
    int size;
    double *array1;
    double *array2;
    int row_start;
    int num_rows;
    bool keep_iterating;
    pthread_mutex_t lock;
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
    while((*arraySection).keep_iterating) {
        printf("Thread on row %d start iterating\n", (*arraySection).row_start);
        //iterating array1 and storing result in array2
        for (int i = (*arraySection).row_start; i < (*arraySection).row_start + (*arraySection).num_rows; i++) {
            for (int j = 1; j < (*arraySection).size; j++) {
                index = get_array_index(i, j, (*arraySection).size);
                (*arraySection).array2[index] = average_value((*arraySection).array1, index, (*arraySection).size);
            }
        }
        printf("Thread on row %d iterated now waiting\n", (*arraySection).row_start);
        pthread_barrier_wait(&barrier1);


        //iterating array2 and storing it in array1
        printf("Thread on row %d start iterating\n", (*arraySection).row_start);
        for (int i = (*arraySection).row_start; i < (*arraySection).row_start + (*arraySection).num_rows; i++) {
            for (int j = 1; j < (*arraySection).size; j++) {
                index = get_array_index(i, j, (*arraySection).size);
                (*arraySection).array1[index] = average_value((*arraySection).array2, index, (*arraySection).size);
            }
        }
        printf("Thread on row %d iterated now waiting\n", (*arraySection).row_start);
        pthread_barrier_wait(&barrier2);
    }
}

void copy_square(double *to_copy, double *new_array, int size){
    int index;
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            index = get_array_index(i, j, size);
            new_array[index] = to_copy[index];
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
    printf("Checking if Within Error\n");
    int index;
    double difference;
    for(int i = 1; i < size - 1; i++){
        for(int j = 1; j < size - 1; j++){
            index = get_array_index(i, j, size);
            difference = array[index] - new_array[index];
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

void iterate(int num_threads, double *array1, double *array2, int size, double error_margin, bool print_iterations){
    copy_boundary(array1, array2, size);

    bool keep_iterating = true;
    int k = 0;

    //Creating Threads
    struct doublesArraySection *sections = malloc(num_threads*sizeof(doublesArraySection));

    pthread_t *threads = malloc(num_threads*sizeof(pthread_t));
    partition_array(sections, num_threads, size);
    pthread_barrier_init(&barrier1, NULL, num_threads);
    for(int i = 0; i < num_threads; i++){
        sections[i].array1 = array1;
        sections[i].array2 = array2;
        sections[i].keep_iterating = true;
    }

    printf("Making threads\n");
    for(int i = 0; i < num_threads; i++){
        pthread_create(&threads[i], NULL, average_section, (void*) &sections[i]);
    }

    printf("Threads made\n");

    //Synchronises Threads before starting
    pthread_barrier_wait(&barrier0);
    pthread_barrier_destroy(&barrier0);
    while(keep_iterating){
        printf("Iterating %d\n", k);
        //iterating array1 and storing it in array2
        pthread_barrier_wait(&barrier1);
        printf("all threads hit barrier and done iteration %d\n", k);
        k++;

        if(within_error(array1, array2, error_margin, size)){
            for(int i = 0; i < num_threads; i++){
                sections[i].keep_iterating = false;
                pthread_join(threads[i], NULL);
            }
            free(sections);
            printf("%d iterations\n", k);
            return;
        }
        //If not done iterating, destroys barrier and loads new one
        pthread_barrier_init(&barrier2, NULL, num_threads);
        pthread_barrier_destroy(&barrier1);
        if(print_iterations) {
            printf("Iteration %d\n", k);
            print_square(array2, size);
        }

        //iterating array2 and storing it in array 1
        pthread_barrier_wait(&barrier2);
        k++;
        if(within_error(array2, array1, error_margin, size)){
            for(int i = 0; i < num_threads; i++) {
                sections[i].keep_iterating = false;
                pthread_join(threads[i], NULL);
            }
            free(sections);
            printf("%d iterations\n", k);
            return;
        }
        //If not done iterating, destroys barrier and loads new one
        pthread_barrier_init(&barrier1, NULL, num_threads);
        pthread_barrier_destroy(&barrier2);
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
            num = (rand() % (MAX_VALUE*den));
            to_fill[get_array_index(i, j, size)] = ((double)num)/(double)den;
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

void generate_tests(int num_threads, int num_tests, int MAX_VALUE, int size, double error_margin,
                    bool use_random_array, bool print_iterations, bool print_start_end){
    double *to_test;
    double *new_array;
    long int start_time, end_time, run_time;
    for(int i = 0; i < num_tests; i++){
        to_test = malloc(size*size*sizeof(double));
        new_array = malloc(size*size*sizeof(double));

        if(use_random_array){
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
        iterate(num_threads, to_test, new_array, size, error_margin, print_iterations);
        end_time = (long int)(time(NULL));
        run_time = end_time - start_time;

        printf("Took %ld seconds to run\n", run_time);

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
    double error_margin = 0.1;
    bool print_iterations = true;
    bool print_start_end = true;

    bool use_random_array = false;

    generate_tests(num_threads, num_test, MAX_VALUE, SIZE, error_margin,
                   use_random_array, print_iterations, print_start_end);

    return 0;
}
