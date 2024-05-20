#include <stdio.h>
#include <stdlib.h>

// Function to read a matrix from a binary file and print it
void print_matrix(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(1);
    }
    
    int rows, cols;
    fread(&rows, sizeof(int), 1, file);
    fread(&cols, sizeof(int), 1, file);
    
    printf("Matrix (%dx%d):\n", rows, cols);
    
    float* matrix = (float*)malloc(rows * cols * sizeof(float));
    fread(matrix, sizeof(float), rows * cols, file);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%f\t", matrix[i * cols + j]);
        }
        printf("\n");
    }
    
    free(matrix);
    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }
    
    print_matrix(argv[1]);
    return 0;
}
