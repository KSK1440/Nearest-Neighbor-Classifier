#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <math.h>

#include <pthread.h>



// Structure holding information about a vector

typedef struct {

    float vector[2];

    char label;

} Vector;



Vector *testVectors = NULL;

Vector *trainingVectors = NULL;

int testSize = 0;

int trainingSize = 0;

int hit = 0;

int miss = 0;

int classificationCompleted = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;



// Function loading vectors from a file

void loadVectorsFromFile(const char *filename, Vector **vectors, int *size) {

    FILE *file = fopen(filename, "r");

    if (file == NULL) {

        perror("Error opening file");

        exit(EXIT_FAILURE);

    }



    Vector *tempVectors = NULL;

    int tempSize = 0;

    int capacity = 10;



    tempVectors = malloc(capacity * sizeof(Vector));

    if (tempVectors == NULL) {

        perror("Memory allocation error");

        exit(EXIT_FAILURE);

    }



    while (fscanf(file, "%f,%f,%c", &tempVectors[tempSize].vector[0], &tempVectors[tempSize].vector[1], &tempVectors[tempSize].label) == 3) {

        tempSize++;



        if (tempSize >= capacity) {

            capacity *= 2;

            tempVectors = realloc(tempVectors, capacity * sizeof(Vector));

            if (tempVectors == NULL) {

                perror("Memory reallocation error");

                exit(EXIT_FAILURE);

            }

        }

    }



    fclose(file);



    *vectors = tempVectors;

    *size = tempSize;



    printf("Loaded %d vectors from the file: %s\n", *size, filename);

}



// Function performing vector classification

void classify() {

    printf("Classification process started...\n");



    for (int i = 0; i < testSize; ++i) {

        int nearestIndex = 0;

        float minDistance = INFINITY;



        for (int j = 0; j < trainingSize; ++j) {

            float distance = sqrt(pow(testVectors[i].vector[0] - trainingVectors[j].vector[0], 2) +

                                  pow(testVectors[i].vector[1] - trainingVectors[j].vector[1], 2));



            if (distance < minDistance) {

                minDistance = distance;

                nearestIndex = j;

            }

        }



        pthread_mutex_lock(&mutex);

        if (testVectors[i].label == trainingVectors[nearestIndex].label) {

            hit++;

        } else {

            miss++;

        }

        pthread_mutex_unlock(&mutex);

    }



    // Signaling the completion of classification

    pthread_mutex_lock(&mutex);

    classificationCompleted = 1;

    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);



    printf("Classification process completed.\n");

}



// Function displaying current results in a separate thread

void *displayResults(void *arg) {

    while (1) {

        char choice;

        printf("[R] Display results\n[W] Exit\n");

        scanf(" %c", &choice);



        pthread_mutex_lock(&mutex);

        if (choice == 'R' || choice == 'r') {

            printf("Current result: %d / %d\n", hit, hit + miss);

            if (!classificationCompleted) {

                pthread_cond_wait(&cond, &mutex);  // Waiting for the signal indicating the completion of classification

            }

        } else if (choice == 'E' || choice == 'e') {

            pthread_mutex_unlock(&mutex);

            exit(EXIT_SUCCESS);

        } else {

            printf("Invalid choice. Please try again.\n");

        }

        pthread_mutex_unlock(&mutex);

    }



    return NULL;

}



int main() {

    loadVectorsFromFile("test.csv", &testVectors, &testSize);

    loadVectorsFromFile("training.csv", &trainingVectors, &trainingSize);



    pthread_t resultThread;



    printf("[C] Classify\n[R] Display results\n[E] Exit\n");



    char menuChoice;

    scanf(" %c", &menuChoice);



    switch (menuChoice) {

        case 'C':

        case 'c':

            pthread_create(&resultThread, NULL, displayResults, NULL);

            classify();

            pthread_join(resultThread, NULL);

            break;

        case 'R':

        case 'r':

            pthread_create(&resultThread, NULL, displayResults, NULL);

            pthread_join(resultThread, NULL);

            break;

        case 'E':

        case 'e':

            exit(EXIT_SUCCESS);

        default:

            printf("Invalid choice. Exiting the program.\n");

            exit(EXIT_FAILURE);

    }



    // Free allocated memory

    free(testVectors);

    free(trainingVectors);



    return 0;

}

