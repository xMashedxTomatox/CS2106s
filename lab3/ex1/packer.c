/**
 * CS2106 AY21/22 Semester 1 - Lab 3
 *
 * Name: Tan Chen Xiang
 * Student No: A0223962A
 * Lab Group: 03
 */

#include "packer.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define COLORS 3
#define MAXBALLS 2

// You can declare global variables here
typedef struct BallSet {
  int* ballIds;
  int* count;
} ballSet_t;

sem_t mutexBarrier[COLORS];
sem_t mutex[COLORS];
ballSet_t * ballSets;

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    ballSets = malloc(COLORS * sizeof(ballSet_t));
    for (int i = 0; i < COLORS; i++) {
      sem_init(&mutexBarrier[i], 0, 0);
      sem_init(&mutex[i], 0, 1);
      ballSets[i].ballIds = (int *) malloc(MAXBALLS * sizeof(int));
      ballSets[i].count = (int *) malloc(sizeof(int));
    }
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 0; i < COLORS; i++) {
      sem_destroy(&mutexBarrier[i]);
      sem_destroy(&mutex[i]);
      free(ballSets[i].ballIds);
      free(ballSets[i].count);
    }
    free(ballSets);
}

int pack_ball(int colour, int id) {
    // Write your code here.
    sem_wait(&mutex[colour - 1]);
    ballSet_t* curr_set = &ballSets[colour - 1];
    int* count = curr_set -> count;
    int refCount = *count;
    (*count)++;
    curr_set -> ballIds[(*count) - 1] = id;
    if ((*count) == MAXBALLS)
      sem_post(&mutexBarrier[colour - 1]);
    sem_post(&mutex[colour - 1]);

    sem_wait(&mutexBarrier[colour - 1]);
    sem_post(&mutexBarrier[colour - 1]);
     
    return curr_set -> ballIds[abs(refCount - 1)];
}
