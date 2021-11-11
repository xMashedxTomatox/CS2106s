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
#define PAGE_ENTRY_COUNT 512
#define PAGE_ENTRY_POW 9
#define PAGE_TABLE_LAYER 4
#define LORMS_INIT_PAGES 2016

typedef struct node node_t;
typedef struct page_table page_table_t;

struct node {
  void* addr;
  size_t size;
  node_t* next;
};

struct page_table {
  page_table_t* next;
  int accessed_entries;
  bool init;
  bool accessed;
};

page_table_t highest_level_table;

node_t* list_head = NULL;
node_t* list_tail = NULL;
node_t* resident_pages_tail = NULL;
node_t* resident_pages_head = NULL;

int lorms_max_pages = LORMS_INIT_PAGES;

int resident_count = 0;

int* get_table_idx(long addr) {
  //printf("%ld\n", addr);
  static int idx[PAGE_TABLE_LAYER];
  addr = addr >> PAGE_SIZE_POW;
  for (int i = 0; i < PAGE_TABLE_LAYER; i++) {
    idx[i] = addr & ((1 << PAGE_ENTRY_POW) - 1);
    addr = addr >> PAGE_ENTRY_POW;
  }
  return idx;
}

page_table_t* get_page_entry(void* addr) {
  int* idx = get_table_idx((long)(addr));
  page_table_t* curr = &highest_level_table;
  for (int i = 0; i < PAGE_TABLE_LAYER; i++) {
    curr = &(curr -> next[idx[PAGE_TABLE_LAYER - 1 - i]]);
  }
  return curr;
}

void handle_evict() {
  while(resident_count > lorms_max_pages) {
    resident_count--;
    node_t* ref = resident_pages_head;
    if (resident_pages_head == resident_pages_tail) {
      resident_pages_head = NULL;
      resident_pages_tail = NULL;
    } else {
      resident_pages_head = ref -> next;
    }
    mprotect(ref -> addr, PAGE_SIZE, PROT_NONE);
    page_table_t* entry = get_page_entry(ref -> addr);
    entry -> accessed = false;
    free(ref);
  }
}

void userswap_set_size(size_t size) {
  size_t extra = PAGE_SIZE - (size % PAGE_SIZE);
  size += extra;
  lorms_max_pages = size/PAGE_SIZE;
  handle_evict();
}

bool sigseg_set = false;

bool page_fault_handler(siginfo_t *si) {
  long addr = (long)(si -> si_addr);
  int* idx = get_table_idx(addr);
  addr = addr >> PAGE_SIZE_POW;
  page_table_t* curr = &highest_level_table;
  for (int i = 0; i < PAGE_TABLE_LAYER; i++) {
    int table_idx = idx[PAGE_TABLE_LAYER - 1 - i];
    curr = &(curr -> next[table_idx]);
    if (curr -> init == false) {
      return false;
    }
  }
  addr *= PAGE_SIZE;
  void* new_addr = (void*)addr;

  if (curr -> accessed == false) {
    mprotect(new_addr, PAGE_SIZE, PROT_READ);
    curr -> accessed = true;

    node_t* new_node = malloc(sizeof(node_t));
    new_node -> addr = new_addr; 
    new_node -> size = PAGE_SIZE;
    new_node -> next = NULL;
    if (resident_pages_head == NULL) {
      resident_pages_head = new_node;
      resident_pages_tail = new_node;
    } else {
      resident_pages_tail -> next = new_node;
      resident_pages_tail = new_node;
    }
    resident_count++;
    handle_evict();
  } else {
    mprotect(new_addr, PAGE_SIZE, PROT_READ | PROT_WRITE);
  }

  return true;
}

void sigseg_handler(int signal, siginfo_t *si, void *arg) {
  bool result = page_fault_handler(si);
  if (!result) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = SIG_DFL;
    action.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &action, NULL);
    sigseg_set = false;
  }
}


void init_page_table(page_table_t * ref) {
  ref -> init = true;
  ref -> next = malloc(PAGE_ENTRY_COUNT * sizeof(page_table_t));
  for (int i = 0; i < PAGE_ENTRY_COUNT; i++) {
    ref -> next[i].init = false;
    ref -> next[i].accessed = false;
    ref -> accessed_entries = 0;
  }
}

void insert_entry(int* idx) {
  page_table_t* curr = &highest_level_table;
  page_table_t* prev = NULL;

  for (int i = 0; i < PAGE_TABLE_LAYER; i++) {
    int table_idx = idx[PAGE_TABLE_LAYER - 1 - i];
    if (curr -> init == false) {
      init_page_table(curr);
      if (prev)
        prev -> accessed_entries++;
    }
    prev = curr;
    curr = &(curr -> next[table_idx]);
  }

  if (curr -> init == false) {
    curr -> init = true;
    curr -> next = NULL;
    prev -> accessed_entries++;
  }
}

void increment_idx(int* idx, int size) {
  for (int i = 0; i < size; i++) {
    if (idx[i] < PAGE_ENTRY_COUNT - 1) {
      idx[i] = idx[i] + 1;
      break;
    } else {
      idx[i] = 0;
    }
  }
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
  long logical_addr = (long)(addr);
  int* idx = get_table_idx(logical_addr);
  int pages_count = size/PAGE_SIZE;

  for (int i = 0; i < pages_count; i++) {
    insert_entry(idx);
    increment_idx(idx, PAGE_TABLE_LAYER);
  }

  node_t* new_node = malloc(sizeof(node_t));
  new_node -> addr = addr;
  new_node -> size = size;
  new_node -> next = NULL;

  if (list_head == NULL) {
    list_head = new_node;
    list_tail = new_node;
  } else {
    list_tail -> next = new_node;
    list_tail = new_node;
  }
  return addr;
}

bool remove_recur(page_table_t* curr, int curr_idx, int* idx_table) {
  if (curr_idx == -1) {
    curr -> init = false;
    curr -> accessed = false;
    return true;
  }
  else {
    int next_idx = curr_idx - 1;
    bool removed = remove_recur(&(curr -> next[idx_table[curr_idx]]), next_idx, idx_table);
    if (removed) {
      curr -> accessed_entries--;

      if (curr -> accessed_entries == 0) {
        curr -> init = false;
        free(curr -> next);
        curr -> next = NULL;
      }
    }
    return curr -> accessed_entries == 0;
  }
}

void update_resident_list() {
  node_t* itor = resident_pages_head;
  node_t* prev = NULL;
  while(itor != NULL) {
    int* idx = get_table_idx((long)(itor -> addr));
    page_table_t* ref = &highest_level_table;
    bool error = false;
    for (int i = 0; i < PAGE_TABLE_LAYER; i++) {
      if (ref -> init == false) {
        error = true;
        break;
      }
      ref = &(ref -> next[idx[PAGE_TABLE_LAYER - 1 - i]]);
    }
    if (ref -> init == false)
      error = true;
    if (error == true) {
      node_t* remove = itor;
      resident_count--;
      if (itor == resident_pages_head) {
        if (resident_pages_head == resident_pages_tail) {
          resident_pages_head = NULL;
          resident_pages_tail = NULL;
          itor = NULL;
        } else {
          resident_pages_head = itor -> next;
          itor = itor -> next;
        }
      } else if (itor == resident_pages_tail) {
        resident_pages_tail = prev;
        prev -> next = NULL;
        itor = NULL;
      } else {
        prev -> next = itor -> next;
        itor = itor -> next;
      }
      free(remove);
    } else {
      prev = itor;
      itor = itor -> next;
    }
  }
}

void userswap_free(void *mem) {
  node_t* itor = list_head;
  node_t* prev = NULL;
  while(itor != NULL) {
    if (itor -> addr == mem) {
      break;
    }
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

  int* idx = get_table_idx((long)(mem));
  int page_count = itor -> size / PAGE_SIZE;
  for (int i = 0; i < page_count; i++) {
    remove_recur(&highest_level_table, PAGE_TABLE_LAYER - 1, idx);
    increment_idx(idx, PAGE_ENTRY_COUNT);
  }
  update_resident_list();
  munmap(mem, itor -> size);
  free(itor);
}

void *userswap_map(int fd, size_t size) {
  return NULL;
}
