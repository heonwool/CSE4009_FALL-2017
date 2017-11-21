#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define ENTITY 4000

int A [ENTITY][ENTITY];
int B [ENTITY][ENTITY];
unsigned long long int C [ENTITY][ENTITY];

int NUM_THREADS;

// Struct for multi-thread environment
struct matrix {
	int rowStart; /* row */
	int rowEnd;
	int thr;
};

// Thread Function
void *calculate(void *param); 

int main(int argc, char *argv[]) {
	/*
	 * Environment:
	 *
	 * Ubuntu 17.04 64-bit
	 * Intel Core i3-6100 @ 2.30GHz × 4 
	 *
	 * Elapsed Time:
	 *
	 * Matrix calculation with summation
	 * Thread  1: 355.273856 sec
	 * Thread  2: 173.761364 sec
	 * Thread  4: 184.796211 sec
	 * Thread  8: 191.955796 sec
	 * Thread 10: 194.023709 sec
	 * Thread 20: 198.958588 sec
	 * Thread 30: 200.490060 sec
	 * Thread 40: 201.012681 sec
	 *
	 */

	FILE *fp1, *fp2;
	pthread_t *tid;
	struct matrix *data;

	void **tret;
	unsigned long long int final = 0;

	struct timespec start, end;
	double elapsed;

	int i, j, num_thread;

	if(argc < 3) {
		printf("Usage: ./matrix THREAD_NUM MATRIX_1.txt MATRIX_2.txt\n");
		return 0;
	} 

	NUM_THREADS = atoi(argv[1]);
	tid = (pthread_t *) calloc(NUM_THREADS, sizeof(pthread_t));
	data = (struct matrix *) calloc(NUM_THREADS, sizeof(struct matrix));
	tret = (void **) calloc(NUM_THREADS, sizeof(void *));

	fp1 = fopen(argv[2], "r");
	fp2 = fopen(argv[3], "r");

	clock_gettime(CLOCK_MONOTONIC, &start);

	for(i = 0; i < ENTITY; i++) {
		for(j = 0; j < ENTITY; j++) {
			fscanf(fp1, "%d", &A[i][j]);
			fscanf(fp2, "%d", &B[i][j]);
		}
	}

	fclose(fp1);
	fclose(fp2);

	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsed = (end.tv_sec - start.tv_sec);
	elapsed += (end.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("Elapsed time (Matrix loading): %f sec\n", elapsed);

	printf("Matrix loading completed.\n");

	clock_gettime(CLOCK_MONOTONIC, &start);

	// Assign the threads
	for(i = 0; i < NUM_THREADS; i++) {
		data[i].rowStart = i * (ENTITY / NUM_THREADS);
		data[i].rowEnd = (i + 1) * (ENTITY / NUM_THREADS);
		data[i].thr = i;

		if(i == NUM_THREADS - 1 && data[i].rowEnd != ENTITY) {
			data[i].rowEnd = ENTITY;
		}

		printf("Thread %d will calculate row %d to row %d\n", i, data[i].rowStart, data[i].rowEnd - 1);

		pthread_create(&tid[i], NULL, calculate, &data[i]);
	}

	// Wait for the threads 
	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(tid[i], &tret[i]);
	}

	printf("Matrix multiplication completed.\n");

	for(i = 0; i < NUM_THREADS; i++) {
		final += (unsigned long long int) tret[i];
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsed = (end.tv_sec - start.tv_sec);
	elapsed += (end.tv_nsec - start.tv_nsec) / 1000000000.0;

	// Final result: 1,599,733,421,594,486,293
	printf("Final result: %lld\n", final);
	printf("Elapsed time (Matrix multiplication and summation): %f sec\n", elapsed);

	return 0;
}

// The thread will begin control in this function
void *calculate(void *param) {
	struct matrix *data = param;
	int rowIdx, colIdx, n;
	unsigned long long int sum = 0; 
	unsigned long long int result_sum = 0;

	printf("Thread %d start\n", data->thr);

	for(rowIdx = data->rowStart; rowIdx < data->rowEnd; rowIdx++) {
		for(colIdx = 0; colIdx < ENTITY; colIdx++) {
			sum = 0;
			for(n = 0; n < ENTITY; n++) {
				sum += A[rowIdx][n] * B[n][colIdx];
			}
			result_sum += sum;
		}
	}

	printf("Thread %d end\n", data->thr);

	pthread_exit((void *)result_sum);
}
