#include "userswap.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  const _Bool use_userswap = 1;
  const size_t memory_size = 10 * 1024 * 1024;

  volatile uintptr_t *const mem = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile uintptr_t *const mem2 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile uintptr_t *const mem3 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile uintptr_t *const mem4 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile uintptr_t *const mem5 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  if (!mem || !mem2 || !mem3 || !mem4 || !mem5) {
    return 1;
  }

  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    mem[i] = (uintptr_t)(mem + i);
    mem3[i] = (uintptr_t)(mem3 + i);
  }

  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    mem2[i] = (uintptr_t)(mem2 + i);
    mem4[i] = (uintptr_t)(mem4 + i);
  }
  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    mem5[i] = (uintptr_t)(mem5 + i);
  }

  _Bool failed = 0;
  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    if (mem[i] != (uintptr_t)(mem + i)) {
      printf("Failed at %p: %" PRIxPTR "\n", mem + i, mem[i]);
      failed = 1;
    }
    if (mem2[i] != (uintptr_t)(mem2 + i)) {
      printf("Failed at %p: %" PRIxPTR "\n", mem2 + i, mem2[i]);
      failed = 1;
    }
    if (mem3[i] != (uintptr_t)(mem3 + i)) {
      printf("Failed at %p: %" PRIxPTR "\n", mem3 + i, mem3[i]);
      failed = 1;
    }
    if (mem4[i] != (uintptr_t)(mem4 + i)) {
      printf("Failed at %p: %" PRIxPTR "\n", mem4 + i, mem4[i]);
      failed = 1;
    }
    if (mem5[i] != (uintptr_t)(mem5 + i)) {
      printf("Failed at %p: %" PRIxPTR "\n", mem5 + i, mem5[i]);
      failed = 1;
    }
  }

  if (use_userswap) {
    userswap_free((void *)mem);
    userswap_free((void *)mem2);
    userswap_free((void *)mem3);
    userswap_free((void *)mem4);
    userswap_free((void *)mem5);
  } else {
    free((void *)mem);
    free((void *)mem2);
    free((void *)mem3);
    free((void *)mem4);
    free((void *)mem5);
  }

  return failed;
}
