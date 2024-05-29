#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 12

struct thread_data {
    int id;
    int** a;
    int** b;
    int** result;
    int size;
};

void generateRandomMatrix(int** matrix, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
}

void* cacheFriendlyMatrixMultiply(void* threadarg) {
    struct thread_data* data = (struct thread_data*) threadarg;
    int id = data->id;
    int** a = data->a;
    int** b = data->b;
    int** result = data->result;
    int size = data->size;

    for (int i = id; i < size; i += NUM_THREADS) {
        for (int k = 0; k < size; k++) {
            int temp = a[i][k];
            for (int j = 0; j < size; j++) {
                result[i][j] += temp * b[k][j];
            }
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <matrix_size>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int** matrixA = (int**)malloc(size * sizeof(int*));
    int** matrixB = (int**)malloc(size * sizeof(int*));
    int** result = (int**)malloc(size * sizeof(int*));

    generateRandomMatrix(matrixA, size);
    generateRandomMatrix(matrixB, size);

    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int rc;

    clock_t start, end;
    start = clock();

    for (int i = 0; i < size; i++) {
        matrixA[i] = (int*)malloc(size * sizeof(int));
        matrixB[i] = (int*)malloc(size * sizeof(int));
        result[i] = (int*)calloc(size, sizeof(int));
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        thread_data_array[t].id = t;
        thread_data_array[t].a = matrixA;
        thread_data_array[t].b = matrixB;
        thread_data_array[t].result = result;
        thread_data_array[t].size = size;
        rc = pthread_create(&threads[t], NULL, cacheFriendlyMatrixMultiply, (void*)&thread_data_array[t]);
        if (rc) {
            printf("Error: Unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    end = clock();
    double time_taken = ((double)(end - start)) / (CLOCKS_PER_SEC*10);
    printf("Cache-friendly matrix multiplication took %f seconds\n", time_taken);

    for (int i = 0; i < size; i++) {
        free(matrixA[i]);
        free(matrixB[i]);
        free(result[i]);
    }
    free(matrixA);
    free(matrixB);
    free(result);

    return 0;
}
