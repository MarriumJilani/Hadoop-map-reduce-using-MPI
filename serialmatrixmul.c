#include <stdio.h>
#include <stdlib.h>
#include<math.h>

//serial implementation of matrix multiplication
//we need to specify command line argument 4 for 2^4 (eg)
//code is generic though

int** readMatrixFromFile(int size) {
    char filename[20];
    snprintf(filename, sizeof(filename), "matrix1_2^%d.txt", size);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    int msize=pow(2,size);
    int** matrix = (int**)malloc(msize * sizeof(int*));
    for (int i = 0; i < msize; i++) {
        matrix[i] = (int*)malloc(msize * sizeof(int));
    }

    for (int i = 0; i < msize; i++) {
        for (int j = 0; j < msize; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }

    fclose(file);
    return matrix;
}
int** readMatrixFromFile2(int size) {
    char filename[20];
    snprintf(filename, sizeof(filename), "matrix2_2^%d.txt", size);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    int msize=pow(2,size);
    int** matrix = (int**)malloc(msize * sizeof(int*));
    for (int i = 0; i < msize; i++) {
        matrix[i] = (int*)malloc(msize * sizeof(int));
    }

    for (int i = 0; i < msize; i++) {
        for (int j = 0; j < msize; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }

    fclose(file);
    return matrix;
}


void writeMatrixToFile(int** matrix,int size) {
    
    char filename[]="output.txt";
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    printf("Output for serial matrix multiplication stored in output.txt\n");
    fclose(file);
}

void multiplyMatrices(int** matrix1, int** matrix2, int** result, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                sum += matrix1[i][k] * matrix2[k][j];
            }
            result[i][j] = sum;
        }
    }
}

void freeMatrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Specify size!!\n");
        return 1;
    }

    int size = atoi(argv[1]);

    // Read the matrices from files

    int** matrix1 = readMatrixFromFile(size);
    int** matrix2 = readMatrixFromFile2(size);
    if (matrix1 == NULL || matrix2 == NULL) {
	printf("Matrix Null Error");
        return 1;
    }
     int matrix_size=pow(2,size);
    // Perform matrix multiplication
    int** result = (int**)malloc(matrix_size * sizeof(int*));
    for (int i = 0; i < matrix_size; i++) {
        result[i] = (int*)malloc(matrix_size * sizeof(int));
    }
    multiplyMatrices(matrix1, matrix2, result, matrix_size);

    // Write the result matrix to file
    writeMatrixToFile(result, matrix_size);

    // Clean up
    freeMatrix(matrix1, matrix_size);
    freeMatrix(matrix2, matrix_size);
    freeMatrix(result, matrix_size);

    return 0;
}
