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
#include <string.h>

#define SIZETABLE 5

#include "restaurant.h"

typedef struct node node_t;

struct node {
  sem_t sem;
  group_state * state;
  node_t * next;
};

typedef struct TableInfo {
  bool* available_tables;
  int max_table_count;
  int start_index;
} table_info;


table_info* info;
node_t* q_head = NULL;
sem_t mutex;
// You can declare global variables here

void restaurant_init(int num_tables[5]) {
  // Write initialization code here (called once at the start of the program).
  // It is guaranteed that num_tables is an array of length 5.
  int sum = 0;
  info = malloc(SIZETABLE * sizeof(table_info));
  sem_init(&mutex, 0, 1);
  for (int i = 0; i < SIZETABLE; i++) {
    info[i].start_index = sum;
    info[i].max_table_count = num_tables[i];
    info[i].available_tables = malloc(num_tables[i] * sizeof(bool));
    for (int j = 0; j < num_tables[i]; j++)
      info[i].available_tables[j] = false;//initializing all tables to not occupied
    sum += num_tables[i];
  }
}

void restaurant_destroy(void) {
  // Write deinitialization code here (called once at the end of the program).
  sem_destroy(&mutex);
  for (int i = 0; i < SIZETABLE; i++) {
    free(info[i].available_tables);
  }
  free(info);
}

int request_for_table(group_state *state, int num_people) {
  // Write your code here.
  // Return the id of the table you want this group to sit at.
  sem_wait(&mutex);
  node_t* new_node = malloc(sizeof(node_t));
  new_node -> next = NULL;
  sem_init(&(new_node -> sem), 0, 0); 
  bool has_free = false;
  int idx = 0;
  int relative_idx = 0;

  for (int i = num_people - 1; i < SIZETABLE; i++) {
    for (int j = 0; j < info[i].max_table_count; j++) {
      if (info[i].available_tables[j] == false) {
        has_free = true;
        idx = i;
        relative_idx = j;
        break;
      }
    }
    if (has_free)
      break;
  }

  if (!has_free) {
    state -> number_people = num_people;
    new_node -> state = state;
    if (q_head == NULL) {
      q_head = new_node;
    } else {
      node_t* final_node = q_head;
      while (final_node -> next) {
        final_node = final_node -> next;
      }
      final_node -> next = new_node;
    }
    on_enqueue();
    sem_post(&mutex);
    sem_wait(&(new_node -> sem));
  }
  else {
    info[idx].available_tables[relative_idx] = true;
    state -> assigned_table_count = idx + 1;
    state -> relative_table_idx = relative_idx;
    state -> number_people = num_people;
    on_enqueue();
  }

  int table_no = info[state -> assigned_table_count - 1].start_index + state -> relative_table_idx;

  sem_post(&mutex);
  sem_destroy(&new_node -> sem);
  free(new_node);
  return table_no;
}

void leave_table(group_state *state) {
  // Write your code here.
  sem_wait(&mutex);
  int table_max = state -> assigned_table_count;
  info[state -> assigned_table_count - 1].available_tables[state -> relative_table_idx] = false;
  bool found = false;
  node_t* release_node = NULL;
  if (q_head) {
    node_t* prev = q_head;
    node_t* temp_head = q_head -> next;
    if (q_head -> state -> number_people <= table_max) {
      found = true;
      release_node = q_head;
      q_head = q_head -> next;
    } else {
      while(temp_head) {
        if (temp_head -> state -> number_people <= table_max) {
          found = true;
          release_node = temp_head;
          prev -> next = temp_head -> next;
          break;
        } else {
          prev = temp_head;
          temp_head = temp_head -> next;
        }
      }
    }
  }
  if (found) {
    release_node -> state -> assigned_table_count = table_max;
    release_node -> state -> relative_table_idx = state -> relative_table_idx;
    info[state -> assigned_table_count - 1].available_tables[state -> relative_table_idx] = true;
    sem_post(&(release_node -> sem));
  } else {
    sem_post(&mutex);
  }
}

