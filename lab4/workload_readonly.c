#include "userswap.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <malloc.h>

#define CONST_10MB 10 * 1024 * 1024
#define CONST_20MB 20 * 1024 * 1024
#define CONST_40MB 40 * 1024 * 1024
#define CONST_80MB 80 * 1024 * 1024

void printMem();
void printMemUsage();

int pageTotal = 0;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  printf("--------- init --------\n");
  printMemUsage();

  volatile size_t *const mem = userswap_alloc(CONST_10MB);
  pageTotal += 2560;

  size_t scratch = 0;
  for (size_t i = 0; i < CONST_10MB / sizeof(size_t); ++i) {
    scratch += mem[i];
  }
  printf(" ---------- Single 10MB Region Read ---------\n");
  printMemUsage();

  for (size_t i = 0; i < CONST_10MB / sizeof(uintptr_t); ++i) {
    mem[i] = (uintptr_t)(mem + i);
  }
  printf(" ---------- Single 10MB Region Write ---------\n");
  printMemUsage();

  volatile size_t *const mem2 = userswap_alloc(CONST_20MB);
  volatile size_t *const mem3 = userswap_alloc(CONST_40MB);
  volatile size_t *const mem4 = userswap_alloc(CONST_80MB);
  pageTotal += 33280;

  printf(" ---------- Multi 150MB Region Alloc ---------\n");
  printMemUsage();

  for (size_t i = 0; i < CONST_20MB / sizeof(uintptr_t); ++i) {
    mem2[i] = (uintptr_t)(mem2 + i);
    mem3[i] = (uintptr_t)(mem3 + i);
  }

  printf(" ---------- Multi 150MB Region Write --------\n");
  printMemUsage();

  userswap_free((void *) mem2);
  pageTotal -= 5120;
  printf(" ----------- Freeed 20MB Region ---------\n");
  printMemUsage();

  for (size_t i = 0; i < CONST_80MB / sizeof(size_t); ++i) {
    scratch += mem4[i];
  }

  printf(" ------------ Write 80mb Region -------\n");
  printMemUsage();

  userswap_free((void *) mem3);
  userswap_free((void *) mem4);
  pageTotal -= 28160;
  printf(" ------------ Release to 10MB ---------\n");
  printMemUsage();

  // printMem();
  userswap_free((void *)mem);
  pageTotal -= 2560;

  printf("%zu\n", scratch);
  malloc_trim(0);
  printf( "------------- Full Release -----------\n");
  printMemUsage();

  return 0;
}

void printMemUsage() {
  struct mallinfo mi;
  mi = mallinfo();

  printf("Mapped Bytes :%d bytes\n", mi.hblkhd);
  printf("Allocated Space: %d bytes\n", mi.uordblks);
  if (pageTotal > 0)
    printf("Average Allocation: %.6f bytes\n", ((float)mi.uordblks) / pageTotal); 
}

void printMem() {
  int currRealMem = 0;
  int peakRealMem = 0;
  int currVirtMem = 0;
  int peakVirtMem = 0;

  char buffer[1024] = "";

  FILE *file = fopen("/proc/self/status", "r");

  while (fscanf(file, " %1023s", buffer) == 1) {
    if (strcmp(buffer, "VmRSS:") == 0) {
      if (fscanf(file, " %d", &currRealMem) < 0) {
        printf("cant read\n");
      }
    }
    if (strcmp(buffer, "VmHWM:") == 0) {
      if (fscanf(file, " %d", &peakRealMem) < 0) {
        printf("Cant read\n");
      }
    }
    if (strcmp(buffer, "VmSize:") == 0) {
      if (fscanf(file, " %d", &currVirtMem) < 0) {
        printf("Cant Read\n");
      }
    }
    if (strcmp(buffer, "VmPeak:") == 0) {
      if (fscanf(file, " %d", &peakVirtMem) < 0) {
        printf("Cant Read\n");
      }
    }
  }
  fclose(file);
  printf("Cur Real Mem: %d\nPeak Real Mem: %d\n", currRealMem, peakRealMem);
  printf("Cur Virt Mem: %d\nPeak Virt Mem: %d\n", currVirtMem, peakVirtMem);
}

