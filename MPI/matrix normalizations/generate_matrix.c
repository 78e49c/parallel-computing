#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to generate a random matrix and save it to a binary file
void generate_matrix(const char* filename, int size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(1);
    }
    
    // Write the number of rows and columns
    fwrite(&size, sizeof(int), 1, file);
    fwrite(&size, sizeof(int), 1, file);
    
    // Allocate memory for the matrix
    float* matrix = (float*)malloc(size * size * sizeof(float));
    
    // Seed the random number generator
    srand(time(NULL));
    
    // Fill the matrix with random float values between 0 and 100
    for (int i = 0; i < size * size; i++) {
        matrix[i] = (float)rand() / RAND_MAX * 100.0;
    }
    
    // Write the matrix to the file
    fwrite(matrix, sizeof(float), size * size, file);
    
    // Clean up
    free(matrix);
    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    if (size <= 0) {
        fprintf(stderr, "Size must be a positive integer\n");
        return 1;
    }
    
    generate_matrix("matrix.bin", size);
    return 0;
}
