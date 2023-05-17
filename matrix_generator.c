#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <sched.h>
#include<math.h>
#include <sys/time.h>

void generateRandomMatrix(int filesize,int size) {
    int **matrix = (int **) malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int *) malloc(size * sizeof(int));
    }

    // Generate random values for the matrix
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 10;
        }
    }

    // Save the matrix to a file
    char filename[20];
    snprintf(filename, sizeof(filename), "matrix1_2^%d.txt", filesize);

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Matrix1 of size 2^%d stored in %s file.\n", filesize, filename);

/////for matrix 2
    int **matrix2 = (int **) malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++) {
        matrix2[i] = (int *) malloc(size * sizeof(int));
    }

    // Generate random values for the matrix
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix2[i][j] = rand() % 10;
        }
    }

    // Save the matrix to a file
    char filename2[20];
    snprintf(filename2, sizeof(filename2), "matrix2_2^%d.txt", filesize);

    FILE *file2 = fopen(filename2, "w");
    if (file2 == NULL) {
        printf("Error opening file.\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file2, "%d ", matrix2[i][j]);
        }
        fprintf(file2, "\n");
    }

    fclose(file2);
    printf("Matrix2 of size 2^%d stored in %s file.\n", filesize, filename2);

    // Free the memory
    for (int i = 0; i < size; i++) {
        free(matrix2[i]);
    }
    free(matrix2);

    // Free the memory
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("You must specify the parameter: ./program_name <size>\n");
        return 1;
    }

    int size = atoi(argv[1]);

    // Generate matrix for given size
    if (size >= 4) {
////////////////USE -lm TO COMPILE TO LINK MATH LIBRARY!!
        generateRandomMatrix(size,pow(2,size));

    } else {
        printf("Size must be greater than or equal to 2^4.\n");
        //return 1;
   }

    return 0;
}
