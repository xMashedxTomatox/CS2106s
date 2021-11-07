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

  volatile size_t *const mem = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile size_t *const mem2 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile size_t *const mem3 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile size_t *const mem4 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  volatile size_t *const mem5 = use_userswap ? userswap_alloc(memory_size) : malloc(memory_size);
  if (!mem || !mem2 || !mem3 || !mem4 || !mem5) {
    return 1;
  }

  size_t scratch = 0;
  for (size_t i = 0; i < memory_size / sizeof(size_t); ++i) {
    scratch += mem[i] + mem2[i] + mem3[i] + mem4[i] + mem5[i];
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

  printf("%zu\n", scratch);

  return 0;
}
