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
#include <fcntl.h>
#include <errno.h>

#define UNUSED(x) (void)(x)

#define PAGE_SIZE 4096
#define PAGE_SIZE_POW 12
#define PAGE_ENTRY_COUNT 512
#define PAGE_ENTRY_POW 9
#define PAGE_TABLE_LAYER 4
#define LORMS_INIT_PAGES 2106

#define UNACCESSED 0
#define ACCESSED 1
#define DIRTY 2
#define EVICTED 3

typedef struct node node_t;
typedef struct page_table page_table_t;

struct node {
  void* addr;
  size_t size;
  node_t* next;
};

typedef struct page_entry {
  int state;
  long offset;
  int fd;
} page_entry_t;

struct page_table {
  page_table_t* next;
  page_entry_t entry;
  int accessed_entries;
  bool init;
};

page_table_t highest_level_table;

node_t* list_head = NULL;
node_t* list_tail = NULL;
node_t* resident_pages_tail = NULL;
node_t* resident_pages_head = NULL;
node_t* free_head = NULL;
node_t* free_tail = NULL;

int lorms_max_pages = LORMS_INIT_PAGES;
int resident_count = 0;
long swap_size = 0;
int swap_fd = 0;

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

    page_table_t* entry = get_page_entry(ref -> addr);
    if (entry -> entry.state == DIRTY) {
      int fd = 0;
      size_t offset = 0;
      if (entry -> entry.fd == -1) {
        fd = swap_fd;
        entry -> entry.state = EVICTED;
        if (free_head != NULL) {
          offset = free_head -> size;
          node_t * remove = free_head;
          if (free_head == free_tail) {
            free_head = NULL;
            free_head = NULL;
          } else {
            free_head = free_head -> next;
          }
          free(remove);
        } else {
          offset = swap_size;
          swap_size++;
        }
        entry -> entry.offset = offset;
      } else {
        fd = entry -> entry.fd;
        offset = entry -> entry.offset;
        entry -> entry.state = UNACCESSED;
      }

      if (pwrite(fd, ref -> addr, PAGE_SIZE, offset * PAGE_SIZE) == -1) {
        fprintf(stderr, "\nError %d: Loading from \"testing\" file failed: %s %ld %ld\n", errno, strerror(errno), offset, (long)(ref -> addr));
      }
    } else {
      entry -> entry.state = UNACCESSED;
    } 
    mprotect(ref -> addr, PAGE_SIZE, PROT_NONE);
    madvise(ref -> addr, PAGE_SIZE, MADV_DONTNEED);
    free(ref);
  }
}

void userswap_set_size(size_t size) {
  if (size % PAGE_SIZE != 0)
    size += PAGE_SIZE - (size % PAGE_SIZE);
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

  if (curr -> entry.state == UNACCESSED || curr -> entry.state == EVICTED) {
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
    mprotect(new_addr, PAGE_SIZE, PROT_READ | PROT_WRITE);

    if (curr -> entry.state == EVICTED) {
      if(pread(swap_fd, new_addr, PAGE_SIZE, curr -> entry.offset * PAGE_SIZE) == -1) { 
        fprintf(stderr, "\nError %d: Loading from \"testing\" file failed: %s %ld\n", errno, strerror(errno), (long)(new_addr));
      }
      node_t * new_node = malloc(sizeof(node_t));
      new_node -> size = curr -> entry.offset;
      new_node -> next = NULL;
      if (free_head == NULL) {
        free_head = new_node;
        free_tail = new_node;
      } else {
        free_tail -> next = new_node;
        free_tail = new_node;
      }
      
    } else if (curr -> entry.fd != -1) {
      if(pread(curr -> entry.fd, new_addr, PAGE_SIZE, curr -> entry.offset * PAGE_SIZE) == -1) {
        fprintf(stderr, "\nError %d: Loading from \"testing\" file failed: %s %ld\n", errno, strerror(errno), (long)(new_addr));
      }
    }

    mprotect(new_addr, PAGE_SIZE, PROT_READ);

    curr -> entry.state = ACCESSED;
    handle_evict();
  } else if (curr -> entry.state == ACCESSED) {
    curr -> entry.state = DIRTY;
    mprotect(new_addr, PAGE_SIZE, PROT_READ | PROT_WRITE);
  } 
  return true;
}

void sigseg_handler(int sig, siginfo_t *si, void* arg) {
  UNUSED(sig);
  UNUSED(arg);

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
    ref -> accessed_entries = 0;
  }
}

void insert_entry(int* idx, int fd, int offset) {
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
    curr -> entry.state = UNACCESSED;
    curr -> entry.fd = fd;
    curr -> entry.offset = offset;
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
void init_handler() {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = sigseg_handler;
  sigaction(SIGSEGV, &action, NULL);
  sigseg_set = true;
  pid_t pid = getpid();
  char filename[100];
  sprintf(filename, "%d.swap", pid);
  if (access(filename, F_OK) == 0) 
    remove(filename);
  swap_fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
}

void *userswap_handler(size_t size, int fd) {
  if (sigseg_set == 0) {
    init_handler();
  }

  if (size % PAGE_SIZE != 0)
    size += PAGE_SIZE - (size % PAGE_SIZE);

  if (fd != -1) {
    if(ftruncate(fd, size) == -1) {
      fprintf(stderr, "\nError %d: Extending \"testing\" file failed: %s\n", errno, strerror(errno));
    }
  }

  void* addr = mmap(NULL, size, PROT_NONE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  long logical_addr = (long)(addr);
  int* idx = get_table_idx(logical_addr);
  int pages_count = size/PAGE_SIZE;

  for (int i = 0; i < pages_count; i++) {
    insert_entry(idx, fd, i);
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

void *userswap_alloc(size_t size) {
  return userswap_handler(size, -1); 
}

void *userswap_map(int fd, size_t size) {
  lseek(fd, 0, SEEK_SET);
  return userswap_handler(size, fd);
}
bool remove_recur(page_table_t* curr, int curr_idx, int* idx_table) {
  if (curr_idx == -1) {
    curr -> init = false; 
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

void reset_swap() {
  swap_size = 0;
  node_t* itor = free_head;
  while(itor) {
    node_t* remove = itor;
    itor = itor -> next;
    free(remove);
  }
  free_head = NULL;
  free_tail = NULL;
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
  long addr = (long)(mem);
  int* idx = get_table_idx(addr);
  int page_count = itor -> size / PAGE_SIZE;
  for (int i = 0; i < page_count; i++) {
    page_table_t* curr = get_page_entry((void*)addr);
    if (curr -> entry.state == EVICTED) {
      node_t * new_node = malloc(sizeof(node_t));
      new_node -> size = curr -> entry.offset;
      new_node -> next = NULL;
      if (free_head == NULL) {
        free_head = new_node;
        free_tail = new_node;
      } else {
        free_tail -> next = new_node;
        free_tail = new_node;
      }
    } 
    else if (curr -> entry.state == DIRTY && curr -> entry.fd != -1) {
      if (pwrite(curr -> entry.fd, (void*)addr, PAGE_SIZE, curr -> entry.offset * PAGE_SIZE) == -1) {
        fprintf(stderr, "\nError %d: Writing to \"testing\" file failed: %s %ld %ld\n", errno, strerror(errno), curr -> entry.offset, addr);
      }
    } 
    addr += PAGE_SIZE;
    remove_recur(&highest_level_table, PAGE_TABLE_LAYER - 1, idx);
    increment_idx(idx, PAGE_ENTRY_COUNT);
    //printf("freed: %ld\n", (long)(mem) + i*PAGE_SIZE);
  }

  update_resident_list();
  if (resident_count == 0)
    reset_swap();
  munmap(mem, itor -> size);
  free(itor);
}

