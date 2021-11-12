/**
 * CS2106 AY21/22 Semester 1 - Lab 4
 *
 * Name: Tan Chen Xiang
 * Student No: A0223962A
 * Lab Group: 03
 */

#include "userswap.h"
#include<sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

#define PAGE_SIZE 4096
#define PAGE_SIZE_POW 12

typedef struct node node_t;

struct node {
  void* addr;
  size_t size;
  node_t* next;
};

node_t* list_head = NULL;
node_t* list_tail = NULL;


void userswap_set_size(size_t size) {
  
}

bool sigseg_set = false;

void page_fault_handler(siginfo_t *si) {
  long addr = (long)(si -> si_addr);
  addr = addr >> PAGE_SIZE_POW;
  addr *= PAGE_SIZE;
  void* new_addr = (void*)addr;
  mprotect(new_addr, PAGE_SIZE, PROT_READ);
}

void sigseg_handler(int signal, siginfo_t *si, void *arg) {
  page_fault_handler(si);
}

void *userswap_alloc(size_t size) {
  if (sigseg_set == 0) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigseg_handler;
    sigaction(SIGSEGV, &action, NULL);
    sigseg_set = true;
  }
  
  size_t extra = PAGE_SIZE - (size % PAGE_SIZE);
  size += extra;
  
  void* addr = mmap(NULL, size, PROT_NONE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  node_t* new_node = malloc(sizeof(node_t));
  new_node -> addr = addr;
  new_node -> size = size;

  if (list_head == NULL) {
    list_head = new_node;
    list_tail = new_node;
  } else {
    list_tail -> next = new_node;
    list_tail = new_node;
  }
  return addr;
}

void userswap_free(void *mem) {
  node_t* itor = list_head;
  node_t* prev = NULL;
  while(itor != NULL) {
    if (itor -> addr == mem)
      break;
    prev = itor;
    itor = itor -> next;
  }

  if (itor == NULL)
     return;
  else if (itor == list_head) {
    if (list_head == list_tail) {
      list_tail = NULL;
    }
    list_head = itor -> next;
  } else {
    if (itor == list_tail) {
      list_tail = prev;
    }
    prev -> next = itor -> next;
  }

  munmap(mem, itor->size);
  free(itor);
}

void *userswap_map(int fd, size_t size) {
  return NULL;
}
