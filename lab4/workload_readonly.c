#include "userswap.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

void stopwatchStart();
void stopwatchStop();

struct timeval stopwatch;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  const size_t memory_size = 1024 * 1024 * 1024;

  stopwatchStart();
  volatile size_t *const mem = userswap_alloc(memory_size);
  if (!mem) {
    return 1;
  }

  size_t scratch = 0;
  for (size_t i = 0; i < memory_size / sizeof(size_t); ++i) {
    scratch += mem[i];
  }

  userswap_free((void *)mem);
  printf("------ 1GB Case ------\n");
  stopwatchStop();

  stopwatchStart();
  volatile size_t *const mem2 = userswap_alloc(5 * memory_size);
  if (!mem2) {
    return 1;
  }

  for (size_t i = 0; i < 5 * memory_size / sizeof(size_t); ++i) {
    scratch += mem2[i];
  }

  userswap_free((void *)mem2);
  printf("------ 5GB Case ------\n");
  stopwatchStop();

  return 0;
}

void stopwatchStart() {
  gettimeofday(&stopwatch, NULL);
}

void stopwatchStop() {
  struct timeval stop;
  gettimeofday(&stop, NULL);
  printf("Took %lu ms\n", (stop.tv_sec - stopwatch.tv_sec) * 1000 + (stop.tv_usec - stopwatch.tv_usec) / 1000);
}
