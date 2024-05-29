#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

typedef struct {
  int start_term;
  int end_term;
  double* partial_sum; // Pointer to partial sum
} thread_data;

double taylor_series_term(int n);  // Function declaration

void calculate_pi_part(void* arg) {
  thread_data* data = (thread_data*)arg;
  double* partial_sum = (double*)malloc(sizeof(double)); // Allocate memory for partial sum
  *partial_sum = 0.0; // Initialize sum to 0

  for (int i = data->start_term; i <= data->end_term; i++) {
    *partial_sum += taylor_series_term(i);
  }

  // Update the original structure with the computed sum
  *(data->partial_sum) = *partial_sum;

  free(partial_sum); // Free allocated memory
}

double calculate_pi(int total_terms, int num_threads) {
  thread_data data[num_threads];

  // Adjust thread data
  for (int i = 0; i < num_threads; i++) {
    int start_term = i * (total_terms / num_threads);
    int end_term = (i + 1) * (total_terms / num_threads) - 1;

    if (i == num_threads - 1) {
      // Captures remaining terms for the last thread
      end_term = total_terms - 1;
    }

    data[i].start_term = start_term;
    data[i].end_term = end_term;
    data[i].partial_sum = (double*)malloc(sizeof(double)); // Allocate memory for partial sum
  }

  // Create and run threads
  pthread_t threads[num_threads];
  clock_t start_time, end_time;
  double elapsed_time;

  start_time = clock(); // Start timing

  for (int i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, (void* (*)(void*))calculate_pi_part, &data[i]); // Pass address of data[i]
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  end_time = clock(); // End timing
  elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  // Sum partial sums and calculate pi
  double pi_estimate = 0.0;
  for (int i = 0; i < num_threads; i++) {
    pi_estimate += *(data[i].partial_sum);
    free(data[i].partial_sum); // Free allocated memory
  }
  pi_estimate *= 4;

  //printf("Number of Threads: %d\n", num_threads);
  //printf("Total Terms: %d\n", total_terms);
  printf("Elapsed Time: %.6f seconds\n", elapsed_time);

  return pi_estimate;
}

// You need to implement this function to calculate a term in the Taylor series
double taylor_series_term(int n) {
  double denominator = 2 * n + 1;
  if (n % 2 == 0) {
    return 1.0 / denominator;
  } else {
    return -1.0 / denominator;
  }
}

int main() {
  int total_terms;
  int num_threads;

  printf("Enter the total number of terms: ");
  scanf("%d", &total_terms);

  printf("Enter the number of threads to use: ");
  scanf("%d", &num_threads);

  double pi = calculate_pi(total_terms, num_threads);

  printf("Estimated Pi Value: %.10f\n", pi);

  return 0;
}
