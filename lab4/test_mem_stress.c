#include "userswap.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <malloc.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONST_10MB 10 * 1024 * 1024
#define CONST_20MB 20 * 1024 * 1024
#define CONST_40MB 40 * 1024 * 1024
#define CONST_80MB 80 * 1024 * 1024
#define CONST_1GB 1024 * 1024 * 1024

void printMem();
void stopwatchStart();
void stopwatchStop();
void expectSwapSize(long size);

int pageTotal = 0;
struct timeval stopwatch;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  char msg[50];

  printf("[WARN] THIS TEST WRITES A HUGE FILE, TEST ON XCNE. PRESS ENTER TO CONTINUE\n");
  if (fgets(msg, 50, stdin) == 0) {}

  printf("--------- init --------\n");
  printMem();

  stopwatchStart();
  volatile size_t *const mem = userswap_alloc(CONST_1GB);
  pageTotal += 262144;

  size_t scratch = 0;
  for (size_t i = 0; i < CONST_1GB / sizeof(size_t); ++i) {
    scratch += mem[i];
  }
  printf(" ---------- Single 1GB Region Read ---------\n");
  stopwatchStop();
  printMem();

  volatile size_t *const mem2 = userswap_alloc(CONST_10MB);
  pageTotal += 2560;

  // Scratch Mem 2, takes 2560 Pages, 2106 in mem, swap = 454
  for (size_t i = 0; i < CONST_10MB / sizeof(uintptr_t); ++i) {
    mem2[i] = (uintptr_t)(mem2 + i);
  }
  printf(" ---------- Scratch 10MB ---------\n");
  expectSwapSize(1859584);
  printMem();
  userswap_free((void *)mem2);
  pageTotal -= 2560;

  // Scratch Mem 3, takes 5120 Pages, 2106 in mem, swap = 3014
  volatile size_t *const mem3 = userswap_alloc(CONST_20MB);
  pageTotal += 5120;
  for (size_t i = 0; i < CONST_20MB / sizeof(uintptr_t); ++i) {
    mem3[i] = (uintptr_t)(mem3 + i);
  }
  printf(" ---------- Scratch 20MB ---------\n");
  expectSwapSize(12345344);
  printMem();
  
  // Flush mem 3 to swap
  for (size_t i = 0; i < CONST_10MB / sizeof(size_t); ++i) {
    scratch += mem[i];
  }
  printf(" ---------- Flushed 20MB ---------\n");
  expectSwapSize(20971520);
  printMem();
  userswap_free((void *)mem3);
  pageTotal -= 5120;

  // LORMS Mem 4 Check
  volatile size_t *const mem4 = userswap_alloc(CONST_40MB);
  pageTotal += 10240;

  for (size_t i = 0; i < CONST_20MB / sizeof(uintptr_t); ++i) {
    mem4[i] = (uintptr_t)(mem4 + i);
  }

  for (size_t i = 0; i < CONST_10MB / sizeof(size_t); ++i) {
    scratch += mem[i];
  }
  
  printf(" ---------- 40MB Flush Check ---------\n");
  expectSwapSize(20971520);
  printMem();

  userswap_set_size(CONST_20MB);
  pageTotal += 5120;

  for (size_t i = CONST_20MB / sizeof(uintptr_t); i < CONST_40MB / sizeof(uintptr_t); ++i) {
    mem4[i] = (uintptr_t)(mem4 + i);
  }

  printf(" ---------- 40MB Extend LORMS ---------\n");
  expectSwapSize(20971520);
  printMem();

  userswap_set_size(CONST_10MB);

  printf(" ---------- 40MB Contract LORMS 1 ---------\n");
  expectSwapSize(31457280);
  printMem();

  userswap_set_size(4 * 1024 * 1024);

  printf(" ---------- 40MB Contract LORMS 2 ---------\n");
  expectSwapSize(37748736);
  printMem();

  userswap_set_size(CONST_40MB);
  volatile size_t *const mem5 = userswap_alloc(CONST_20MB);
  for (size_t i = 0; i < CONST_20MB / sizeof(uintptr_t); ++i) {
    mem5[i] = (uintptr_t)(mem5 + i);
  }

  printf(" ---------- Mem 5 Expand LORMS ---------\n");
  expectSwapSize(37748736);
  printMem();

  userswap_free((void *)mem4);
  pageTotal -= 10240;

  printf(" ---------- Mem 5 Flush ---------\n");
  for (size_t i = 0; i < CONST_40MB / sizeof(size_t); ++i) {
    scratch += mem[i];
  }
  expectSwapSize(37748736);
  printMem();

  userswap_free((void *) mem5);
  pageTotal -= 5120;
  userswap_free((void *) mem);
  pageTotal -= 262144;


  printf(" ---------- Release All ---------\n");
  printMem();

  return 0;
}

void printMem() {
  struct mallinfo mi;
  mi = mallinfo();

  printf("Allocated Space: %d bytes\n", mi.uordblks);
  if (pageTotal > 0)
    printf("Average Allocation: %.6f bytes\n", ((float)mi.uordblks) / pageTotal); 
}

void stopwatchStart() {
  gettimeofday(&stopwatch, NULL);
}

void stopwatchStop() {
  struct timeval stop;
  gettimeofday(&stop, NULL);
  printf("Took %lu ms\n", (stop.tv_sec - stopwatch.tv_sec) * 1000 + (stop.tv_usec - stopwatch.tv_usec) / 1000);
}

void expectSwapSize(long size) {
  char buf[1024];
  struct stat fileDat;

  sprintf(buf, "%d.swap", getpid());
  stat(buf, &fileDat);

  if (fileDat.st_size != size) {
    printf("[FAIL SIZE] Expected swap to be %ld, got %ld\n", size, fileDat.st_size);
  }
}
