/**
 * CS2106 AY21/22 Semester 1 - Lab 3
 *
 * Name: Tan Chen Xiang
 * Student No: A0223962A
 * Lab Group: 03
 */

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZETABLE 5

#include "restaurant.h"

typedef struct node node_t;

struct node {
  sem_t sem;
  group_state* state;
  node_t * next;
};

typedef struct TableInfo {
  node_t * head;
  node_t * tail;

  bool* available_tables;
  int max_table_count;
  int start_index;
} table_info;

table_info* info;
sem_t mutex[SIZETABLE];
// You can declare global variables here

void restaurant_init(int num_tables[5]) {
  // Write initialization code here (called once at the start of the program).
  // It is guaranteed that num_tables is an array of length 5.
  int sum = 0;
  info = malloc(SIZETABLE * sizeof(table_info));
  for (int i = 0; i < SIZETABLE; i++) {
    sem_init(&mutex[i], 0, 1);
    info[i].start_index = sum;
    info[i].max_table_count = num_tables[i];
    info[i].available_tables = malloc(num_tables[i] * sizeof(bool));
    for (int j = 0; j < num_tables[i]; j++)
      info[i].available_tables[j] = false;//initializing all tables to not occupied
    info[i].head = NULL;
    info[i].tail = NULL;
    sum += num_tables[i];
  }
}

void restaurant_destroy(void) {
  // Write deinitialization code here (called once at the end of the program).
  for (int i = 0; i < SIZETABLE; i++) {
    sem_destroy(&mutex[i]);
    free(info[i].available_tables);
  }
  free(info);
}

int request_for_table(group_state *state, int num_people) {
  // Write your code here.
  // Return the id of the table you want this group to sit at.
  sem_wait(&mutex[num_people - 1]);
  node_t* new_node = malloc(sizeof(node_t));
  //state = malloc(sizeof(group_state));
  new_node -> next = NULL;
  sem_init(&(new_node -> sem), 0, 0);
  bool found = false;

  int relative_idx = 0;

  for (int i = 0; i < info[num_people - 1].max_table_count; i++) {
    if (info[num_people - 1].available_tables[i] == false) {
      relative_idx = i;
      found = true;
      break;
    }
  }

  if (!found) {
    new_node -> state = state;
    state -> number_people = num_people;
    if (info[num_people - 1].head == NULL) {
      info[num_people - 1].head = new_node;
      info[num_people - 1].tail = new_node;
    } else {
      info[num_people - 1].tail -> next = new_node;
      info[num_people - 1].tail = new_node;
    }
    on_enqueue();
    sem_post(&mutex[num_people - 1]);
    sem_wait(&(new_node -> sem));
  }
  else {
    info[num_people - 1].available_tables[relative_idx] = true;
    state -> number_people = num_people;
    state -> relative_table_idx = relative_idx;
    on_enqueue();
  }

  int table_no = info[state -> number_people - 1].start_index + state -> relative_table_idx;

  sem_post(&mutex[num_people - 1]);
  sem_destroy(&new_node -> sem);
  free(new_node);
  return table_no;
}

void leave_table(group_state *state) {
  // Write your code here.
  sem_wait(&mutex[state -> number_people - 1]);
  int idx = state -> number_people - 1;
  info[idx].available_tables[state -> relative_table_idx] = false;
  if (info[idx].head == NULL) {
    sem_post(&mutex[state -> number_people - 1]);
  } else {
    info[idx].available_tables[state -> relative_table_idx] = true;
    node_t * next_cust = info[idx].head;
    info[idx].head = info[idx].head -> next;
    next_cust -> state -> relative_table_idx = state -> relative_table_idx;
    sem_post(&next_cust -> sem);
  }
  //free(state);
}
