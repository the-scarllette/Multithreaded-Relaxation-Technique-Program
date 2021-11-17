]#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <pthread.h>

pthread_barrier_t barrier; //Used to synchronise threads

/* Each thread has a section of the array that it works on.
 * A struct is given to each thread that tells it what array section to work.
 * The struct also has locks used for thread synchronisation.
 */
struct doublesArraySection{
    int size;
    double *array1;
    double *array2;
    int row_start;
    int num_rows;
    bool keep_iterating;
    pthread_mutex_t lock0;
    pthread_mutex_t lock1;
    pthread_mutex_t lock2;
} doublesArraySection;

int get_array_index(int i, int j, int size){
    return size*i + j;
}

double average_value(double *array, int index, int size){
    return (array[index - 1] + array[index + 1] + array[index - size] + array[index + size])/4.0;
}

/* The function run by threads. Each thread has a section of the array that it works on.
 */
void* average_section(void *sectionaddress){
    struct doublesArraySection *arraySection = sectionaddress;
    int index;

    while((*arraySection).keep_iterating) {
        //Checks if the thread needs to keep iterating
        pthread_mutex_lock(&(*arraySection).lock0); //Tries to access the lock and waits her until it can
        if(!(*arraySection).keep_iterating){
            pthread_mutex_unlock(&(*arraySection).lock0);
            return 0;
        }
        pthread_mutex_unlock(&(*arraySection).lock0);

        //iterating array1 and storing result in array2
        for (int i = (*arraySection).row_start; i < (*arraySection).row_start + (*arraySection).num_rows; i++) {
            for (int j = 1; j < (*arraySection).size - 1; j++) {
                index = get_array_index(i, j, (*arraySection).size);
                (*arraySection).array2[index] = average_value((*arraySection).array1, index, (*arraySection).size);
            }
        }
        pthread_barrier_wait(&barrier); //Synchronising
        pthread_mutex_lock(&(*arraySection).lock1); //Tries to access lock and waits here until it can.
        //Here the thread is paused, waiting for the main thread to check if the arrays are within the error margin.
        pthread_mutex_unlock(&(*arraySection).lock1);

        //After iterating, checks if it needs to keep iterating.
        pthread_mutex_lock(&(*arraySection).lock0); //Waits here until lock is available
        if(!(*arraySection).keep_iterating){
            pthread_mutex_unlock(&(*arraySection).lock0);
            return 0;
        }
        pthread_mutex_unlock(&(*arraySection).lock0);


        //iterating array2 and storing it in array1
        for (int i = (*arraySection).row_start; i < (*arraySection).row_start + (*arraySection).num_rows; i++) {
            for (int j = 1; j < (*arraySection).size - 1; j++) {
                index = get_array_index(i, j, (*arraySection).size);
                (*arraySection).array1[index] = average_value((*arraySection).array2, index, (*arraySection).size);
            }
        }
        pthread_barrier_wait(&barrier);
        pthread_mutex_lock(&(*arraySection).lock2);
        //Here the thread is paused, waiting for the main thread to check if the arrays are within the error margin.
        pthread_mutex_unlock(&(*arraySection).lock2);
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

/* Takes a declared array and writs it to a given memory address
 */
void write_to_memory(double *write_to, int size, double to_write[size][size]){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            write_to[get_array_index(i, j, size)] = to_write[i][j];
        }
    }
}

/*Takes two array addresses and checks if all values are within a given error margin.
 * This is done to decide if iterating is complete.
 * */
bool within_error(double *array, double *new_array, double error_margin, int size){
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

/*Divides the array into sections of rows.
 * Each thread works on a different section of the array.
 * Takes an address for array sections and a number of sections to makes
 * Divides the array into each of these sections
 * */
void partition_array(struct doublesArraySection *sections, int num_sections, int array_size){
    int rows_per_thread = (array_size - 2)/num_sections;
    for(int k = 0; k < num_sections; k++){
        sections[k].size = array_size;
        sections[k].row_start = k*rows_per_thread + 1;
        sections[k].num_rows = rows_per_thread;
    }
    //If the array annot be perfectly divided, the ecess rows are giving to the last section
    sections[num_sections - 1].num_rows = (array_size - 1) - sections[num_sections - 1].row_start;
}

/*
 * Iterates an array until within error and returns a memory address to the iterated array
 *
 * The iteration works by iterating one array and atoring it in the other,
 * then swapping so iterating in the other and storing the values back into the first array
 * */
double* iterate(int num_threads, double *array1, double *array2, int size, double error_margin){
    //Initiating Values
    copy_boundary(array1, array2, size);
    bool keep_iterating = true;
    //Each thread has an array section
    struct doublesArraySection *sections = malloc(num_threads*sizeof(doublesArraySection));
    pthread_t *threads = malloc(num_threads*sizeof(pthread_t));
    partition_array(sections, num_threads, size); //Divides the array into sections for each thread to have
    pthread_barrier_init(&barrier, NULL, num_threads + 1); //Creates barrier

    //Creates the sections that each thread works on
    for(int i = 0; i < num_threads; i++){
        sections[i].array1 = array1;
        sections[i].array2 = array2;
        sections[i].keep_iterating = true;
        pthread_mutex_init(&sections[i].lock0, NULL);
        pthread_mutex_init(&sections[i].lock1, NULL);
        pthread_mutex_init(&sections[i].lock2, NULL);
        pthread_mutex_lock(&sections[i].lock1);
    }

    //making threads
    for(int i = 0; i < num_threads; i++){
        pthread_create(&threads[i], NULL, average_section, (void*) &sections[i]);
    }

    //Iterating
    while(keep_iterating){
        //iterating array1 and storing it in array2
        pthread_barrier_wait(&barrier); //Waiting for threads to finish an iteration
        //While the threads are stopped, checks if the array needs to be iterated again.
        if(within_error(array2, array1, error_margin, size)){
            //If they are finished, tells each of the threads to stop iterating and joins them
            for(int i = 0; i < num_threads; i++) {
                pthread_mutex_lock(&sections[i].lock0);
                sections[i].keep_iterating = false;
                pthread_mutex_unlock(&sections[i].lock0);
                pthread_mutex_unlock(&sections[i].lock1);
            }
            for(int i = 0; i < num_threads; i++){
                pthread_join(threads[i], NULL);
            }
            free(sections);
            pthread_barrier_destroy(&barrier);
            return array2;
        }
        //If not done iterating, unlocks the threads so they can keep iterating
        for(int i = 0; i< num_threads; i++){
            pthread_mutex_lock(&sections[i].lock2); //Sets up lock for next error check
        }
        for(int i = 0; i < num_threads; i++){
            pthread_mutex_unlock(&sections[i].lock1); //Unlocks threads to they can keep iterating
        }

        //iterating array2 and storing it in array 1
        pthread_barrier_wait(&barrier); //Waiting for the threads to finish an iteration
        //While the threads are stopped, checks if the array needs to be iterated again.
        if(within_error(array2, array1, error_margin, size)){
            //If they are finished, tells each of the threads to stop iterating and joins them
            for(int i = 0; i < num_threads; i++) {
                pthread_mutex_lock(&sections[i].lock0);
                sections[i].keep_iterating = false;
                pthread_mutex_unlock(&sections[i].lock0);
                pthread_mutex_unlock(&sections[i].lock2);
            }
            for(int i = 0; i < num_threads; i++){
                pthread_join(threads[i], NULL);
            }
            free(sections);
            pthread_barrier_destroy(&barrier);
            return array1;
        }
        //If not done iterating, unlocks the threads so they can keep iterating
        for(int i = 0; i< num_threads; i++){
            pthread_mutex_lock(&sections[i].lock1); //Sets up lock for next error check
        }
        for(int i = 0; i < num_threads; i++){
            pthread_mutex_unlock(&sections[i].lock2); //Unlocks threads to they can keep iterating
        }
    }
    free(sections);
}

int main(){
    //Initiating Values
    int size = 3;
    double array[3][3] = {{1.0, 1.0, 1.0},
                          {2.0, 0.0, 2.0},
                          {1.0, 1.0, 1.0}};
    int num_threads = 4;
    double error_margin = 0.01;

    //Allocating Memory
    double *array1, *array2, *final_array;
    array1 = malloc(size*size*sizeof(double));
    array2 = malloc(size*size*sizeof(double));
    final_array = malloc(size*size*sizeof(double));
    write_to_memory(array1, size, array);

    //Iterating Array
    final_array = iterate(num_threads, array1, array2, size, error_margin);

    //Printing Result
    print_square(final_array, size);

    //Freeing Memory
    free(array1);
    free(array2);
    return 0;
}
