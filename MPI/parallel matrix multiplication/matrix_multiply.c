#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Function to read a matrix from a binary file
void read_matrix(const char* filename, int* rows, int* cols, int** matrix_data) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error occurred while opening the file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Read the number of rows and columns
    fread(rows, sizeof(int), 1, file);
    fread(cols, sizeof(int), 1, file);

    // Allocate memory for the matrix data
    *matrix_data = (int*)malloc((*rows) * (*cols) * sizeof(int));
    if (*matrix_data == NULL) {
        printf("Memory allocation error for matrix data!\n");
        MPI_Finalize();
        exit(1);
    }

    // Read the matrix data
    fread(*matrix_data, sizeof(int), (*rows) * (*cols), file);
    
    fclose(file);
}

// Function to write a matrix to a binary file
void write_matrix(const char* filename, int rows, int cols, int* matrix_data) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error occurred while opening the file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Write the number of rows and columns
    fwrite(&rows, sizeof(int), 1, file);
    fwrite(&cols, sizeof(int), 1, file);

    // Write the matrix data
    fwrite(matrix_data, sizeof(int), rows * cols, file);
    
    fclose(file);
}

// Function to print a matrix
void print_matrix(int rows, int cols, int* matrix_data) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d\t", matrix_data[i * cols + j]);
        }
        printf("\n");
    }
}

// Function to perform matrix multiplication
void matrix_multiplication(int* local_matrix_A, int* matrix_B, int* local_result_matrix, int sendcounts, int a_cols, int b_cols) {
    for (int i = 0; i < sendcounts / a_cols; i++) {
        for (int j = 0; j < b_cols; j++) {
            for (int k = 0; k < a_cols; k++) {
                local_result_matrix[i * b_cols + j] += local_matrix_A[i * a_cols + k] * matrix_B[k * b_cols + j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int process_rank, process_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_size);

    // Check the number of arguments
    if (argc != 3) {
        if (process_rank == 0) {
            printf("Usage: mpirun --oversubscribe -np 4 %s A.bin B.bin\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const char* file_A = argv[1];
    const char* file_B = argv[2];

    int a_rows, a_cols, b_rows, b_cols;
    int *matrix_A = NULL, *matrix_B = NULL;

    // Read the matrices from the files
    if (process_rank == 0) {
        read_matrix(file_A, &a_rows, &a_cols, &matrix_A);
        read_matrix(file_B, &b_rows, &b_cols, &matrix_B);
    }

    // Broadcast the number of rows and columns
    MPI_Bcast(&a_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&a_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for the matrices
    if (process_rank != 0) {
        matrix_A = (int*)malloc(a_rows * a_cols * sizeof(int));
        matrix_B = (int*)malloc(b_rows * b_cols * sizeof(int));
        if (matrix_A == NULL || matrix_B == NULL) {
            printf("Memory error!\n");
            MPI_Finalize();
            exit(1);
        }
    }

    // Broadcast the matrices
    MPI_Bcast(matrix_A, a_rows * a_cols, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(matrix_B, b_rows * b_cols, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the chunk size and the remainder
    int chunk_size = a_rows / process_size;
    int remainder = a_rows % process_size;

    // Allocate memory for sendcounts, displs, recvcounts, and recvdispls
    int *sendcounts = (int*)malloc(process_size * sizeof(int));
    int *displs = (int*)malloc(process_size * sizeof(int));
    int *recvcounts = (int*)malloc(process_size * sizeof(int));
    int *recvdispls = (int*)malloc(process_size * sizeof(int));

    // Calculate sendcounts and displs
    int sum = 0;
    for (int i = 0; i < process_size; i++) {
        sendcounts[i] = (chunk_size + (i < remainder ? 1 : 0)) * a_cols;
        displs[i] = sum;
        sum += sendcounts[i];
    }

    // Calculate recvcounts and recvdispls
    sum = 0;
    for (int i = 0; i < process_size; i++) {
        recvcounts[i] = sendcounts[i] / a_cols * b_cols;
        recvdispls[i] = sum;
        sum += recvcounts[i];
    }

    // Allocate memory for the local matrices
    int *local_matrix_A = (int*)malloc(sendcounts[process_rank] * sizeof(int));
    int *local_result_matrix = (int*)calloc(sendcounts[process_rank], sizeof(int));
    if (local_matrix_A == NULL || local_result_matrix == NULL) {
        printf("Memory error!\n");
        MPI_Finalize();
        exit(1);
    }

    // Scatter the matrix A
    MPI_Scatterv(matrix_A, sendcounts, displs, MPI_INT, local_matrix_A, sendcounts[process_rank], MPI_INT, 0, MPI_COMM_WORLD);

    // Perform matrix multiplication
    matrix_multiplication(local_matrix_A, matrix_B, local_result_matrix, sendcounts[process_rank], a_cols, b_cols);

    // Allocate memory for the result matrix
    int *result_matrix = NULL;
    if (process_rank == 0) {
        result_matrix = (int*)malloc(a_rows * b_cols * sizeof(int));
        if (result_matrix == NULL) {
            printf("Memory error!\n");
            MPI_Finalize();
            exit(1);
        }
    }

    // Gather the result matrix
    MPI_Gatherv(local_result_matrix, recvcounts[process_rank], MPI_INT, result_matrix, recvcounts, recvdispls, MPI_INT, 0, MPI_COMM_WORLD);

    // Print and write the result matrix
    if (process_rank == 0) {
        //print_matrix(a_rows, b_cols, result_matrix);
        write_matrix("C.bin", a_rows, b_cols, result_matrix);
        free(result_matrix);
    }

    // Free the allocated memory
    free(local_matrix_A);
    free(local_result_matrix);
    if (process_rank == 0) {
        free(matrix_A);
        free(matrix_B);
    }

    free(sendcounts);
    free(displs);
    free(recvcounts);
    free(recvdispls);

    MPI_Finalize();
    return 0;
}
