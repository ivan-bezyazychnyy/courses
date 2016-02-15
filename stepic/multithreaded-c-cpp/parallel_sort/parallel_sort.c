
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>

#include <omp.h>

#define MAX_SIZE 100000

int array[MAX_SIZE];

const char * INPUT_FILE = "input.txt";
const char * OUTPUT_FILE = "output.txt";

void merge(int a[], int first, int middle, int last) {
	int i = first;
	int j = middle;
	int k = first;

	int size = last - first;
	int *temp = (int *) malloc(size * sizeof(int));
	if (temp == NULL) {
		printf("Failed to allocate memory: %s\n", strerror(errno));
		return;
	}

	while (k != last) {
		if (j == last) {
			temp[k - first] = a[i];
			i++;
		} else if (i == middle) {
			temp[k - first] = a[j];
			j++;
		} else if (a[i] <= a[j]) {
			temp[k - first] = a[i];
			i++;
		} else {
			temp[k - first] = a[j];
			j++;
		}
		k++;	
	}

	memcpy(a + first, temp, size * sizeof(int));

	free(temp);
}

void sort_serial(int a[], int first, int last) {
	if (last - first <= 1) {
		return;
	}

	int middle = first + (last - first) / 2;
	sort_serial(a, first, middle);
	sort_serial(a, middle, last);
	merge(a, first, middle, last);
}

void sort_parallel(int a[], int first, int last, int threads) {
	if (threads <= 1 || (last - first) <= 100) {
		sort_serial(a, first, last);
	} else {
		int middle = first + (last - first) / 2;
		#pragma omp parallel sections
		{
			#pragma omp section
			sort_parallel(a, first, middle, threads / 2 + 1);

			#pragma omp section
			sort_parallel(a, middle, last, threads / 2 + 1);
		}
		merge(a, first, middle, last);
	}
}

void save_result(int a[], int size) {
	FILE *f = fopen(OUTPUT_FILE, "w+");
	if (f == NULL) {
		printf("Failed to open output file: %s\n", strerror(errno));
		return;
	}
	for (int i = 0; i < size; i++) {
		fprintf(f, "%d ", a[i]);
	}
	fclose(f);
}

int main(int argc, char **argv) {
	FILE *f = fopen(INPUT_FILE, "r");
	if (f == NULL) {
		printf("Failed to open input file: %s\n", strerror(errno));
		goto exit;
	}

	int size = 0;
	while (!feof(f)) {
		if (size > MAX_SIZE) {
			printf("Too much numbers\n");
			break;
		}
		if (fscanf(f, "%d", &(array[size])) <= 0) {
			break;
		}
		size++;
	}
	printf("Read %d numbers\n", size);

	int num_of_threads = 8; //omp_get_num_threads();
	omp_set_num_threads(num_of_threads);

	printf("Sorting numbers with %d threads\n", num_of_threads);
	sort_parallel(array, 0, size, num_of_threads);
	sort_serial(array, 0, size);

	save_result(array, size);

	fclose(f);

exit:
	return 0;
}