#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function to read a matrix from a binary file
float* read_matrix(const char* filename, int* rows, int* cols) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fread(rows, sizeof(int), 1, file);
    fread(cols, sizeof(int), 1, file);
    
    int size = (*rows) * (*cols);
    float* matrix = (float*)malloc(size * sizeof(float));
    fread(matrix, sizeof(float), size, file);
    fclose(file);
    return matrix;
}

// Function to write a matrix to a binary file
void write_matrix(const char* filename, float* matrix, int rows, int cols) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fwrite(&rows, sizeof(int), 1, file);
    fwrite(&cols, sizeof(int), 1, file);
    fwrite(matrix, sizeof(float), rows * cols, file);
    fclose(file);
}

// Main function to perform matrix normalization using MPI
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        exit(1);
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows, cols;
    float* matrix = NULL;

    if (rank == 0) {
        matrix = read_matrix(argv[1], &rows, &cols);
    }

    // Broadcast the matrix dimensions to all processes
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int total_elements = rows * cols;
    int local_size = total_elements / size + (rank < total_elements % size ? 1 : 0);
    float* local_matrix = (float*)malloc(local_size * sizeof(float));

    // Scatter the matrix to all processes
    int* sendcounts = (int*)malloc(size * sizeof(int));
    int* displs = (int*)malloc(size * sizeof(int));
    int current_displs = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = total_elements / size + (i < total_elements % size ? 1 : 0);
        displs[i] = current_displs;
        current_displs += sendcounts[i];
    }

    MPI_Scatterv(matrix, sendcounts, displs, MPI_FLOAT, local_matrix, local_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Each process finds the local maximum
    float local_max = local_matrix[0];
    for (int i = 1; i < local_size; i++) {
        if (local_matrix[i] > local_max) {
            local_max = local_matrix[i];
        }
    }

    // Find the global maximum
    float global_max;
    MPI_Allreduce(&local_max, &global_max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

    // Normalize the local matrix using the global maximum
    for (int i = 0; i < local_size; i++) {
        local_matrix[i] /= global_max;
    }

    // Gather the normalized matrix back to the root process
    MPI_Gatherv(local_matrix, local_size, MPI_FLOAT, matrix, sendcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "normalized_%s", argv[1]);
        write_matrix(output_file, matrix, rows, cols);
        free(matrix);
    }

    free(local_matrix);
    free(sendcounts);
    free(displs);
    MPI_Finalize();
    return 0;
}
