#include <stdio.h>
#include <stdlib.h>

// Function to read a matrix from a binary file and print it
void readMatrix(char *filename) {
    FILE *file;
    int rows, cols;

    // Open the binary file in read mode
    file = fopen(filename, "rb");

    if (file == NULL) {
        printf("Error opening the file!");
        return;
    }

    // Read the number of rows and columns from the file
    fread(&rows, sizeof(int), 1, file);
    fread(&cols, sizeof(int), 1, file);

    printf("Matrix (%dx%d):\n", rows, cols);

    // Allocate memory for the matrix
    int *matrix = (int *)malloc(rows * cols * sizeof(int));

    // Read the matrix from the file
    fread(matrix, sizeof(int), rows * cols, file);

    // Print the matrix
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d\t", matrix[i * cols + j]);
        }
        printf("\n");
    }

    // Free the memory allocated for the matrix
    free(matrix);

    // Close the file
    fclose(file);
}

int main() {
    char filename[50];

    printf("Enter the name and path of the matrix file to read: ");
    scanf("%s", filename);

    readMatrix(filename);

    return 0;
}
