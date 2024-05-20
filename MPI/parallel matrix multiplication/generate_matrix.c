#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to generate a matrix with random values and write it to a binary file
void generateMatrix(char *filename, int rows, int cols, int minVal, int maxVal, unsigned int seed) {
    FILE *file;
    
    // Open the binary file in write mode
    file = fopen(filename, "wb");

    if (file == NULL) {
        printf("Error opening the file!");
        return;
    }

    // Write the number of rows and columns to the file
    fwrite(&rows, sizeof(int), 1, file);
    fwrite(&cols, sizeof(int), 1, file);

    // Set the seed for random number generation
    srand(seed);

    // Generate random values for the matrix and write them to the file
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int value = rand() % (maxVal - minVal + 1) + minVal;
            fwrite(&value, sizeof(int), 1, file);
        }
    }

    // Close the file
    fclose(file);
}

int main() {
    char filename_A[50], filename_B[50];
    int rows_A, cols_A, cols_B, minVal, maxVal;

    printf("Enter the number of rows for Matrix A: ");
    scanf("%d", &rows_A);
    printf("Enter the number of columns for Matrix A (should be equal to the number of rows for Matrix B): ");
    scanf("%d", &cols_A);

    printf("Enter the number of columns for Matrix B: ");
    scanf("%d", &cols_B);

    printf("Enter the minimum value: ");
    scanf("%d", &minVal);

    printf("Enter the maximum value: ");
    scanf("%d", &maxVal);

    // Automatically generate the file names
    sprintf(filename_A, "A.bin");
    sprintf(filename_B, "B.bin");

    // Use different seed values for Matrix A and Matrix B
    unsigned int seed_A = (unsigned int)time(NULL);
    unsigned int seed_B = (unsigned int)(time(NULL) + 1);

    generateMatrix(filename_A, rows_A, cols_A, minVal, maxVal, seed_A);
    generateMatrix(filename_B, cols_A, cols_B, minVal, maxVal, seed_B);

    printf("Matrix A was successfully generated and saved to the file '%s'.\n", filename_A);
    printf("Matrix B was successfully generated and saved to the file '%s'.\n", filename_B);

    return 0;
}
