#include <malloc.h>
#include <time.h>

void print_square(double *to_test, int size);

int main(){
    srand(time(NULL));
    int size = 3;
    int k = 0;
    

    double *to_test = malloc(size*size*sizeof(double));
    for(int i = 0; i <= size; i++){
        for(int j = 0; j < size; j++){
            to_test[size*i + j] = ((double)rand())/((double)rand());
            k++;
        }
    }

    print_square(to_test, size);
    free(to_test);
    return 0;
};