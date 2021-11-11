#include "userswap.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  const _Bool use_userswap = 1;
  const size_t memory_size = 1024 * 1024 * 1024;

  char msg[50];
  printf("[WARN] THIS TEST WRITES A HUGE FILE, TEST ON XCNE. PRESS ENTER TO CONTINUE\n");
  if (fgets(msg, 50, stdin) == 0) {}

  uintptr_t *const mem = malloc(memory_size);
  if (!mem) {
    return 1;
  }

  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    mem[i] = (uintptr_t)(mem + i);
  }

  char tempfn[] = "/tmp/uswlrdflXXXXXX";
  int tempfile = mkstemp(tempfn);
  if (tempfile == -1) {
    fprintf(stderr, "Failed to make temporary file\n");
    return 1;
  }
  unlink(tempfn);

  if (write(tempfile, mem, memory_size) != memory_size) {
    fprintf(stderr, "Failed to write to temporary file\n");
    return 1;
  }

  uintptr_t *const file_map = use_userswap
                                  ? userswap_map(tempfile, memory_size)
                                  : mmap(NULL, memory_size, PROT_READ, MAP_PRIVATE, tempfile, 0);
  if (!file_map || file_map == MAP_FAILED) {
    fprintf(stderr, "Failed to map file\n");
    return 1;
  }
  _Bool failed = 0;
  for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
    if (mem[i] != file_map[i]) {
      printf("Failed at entry %zu, expected %" PRIxPTR ", got %" PRIxPTR "\n", i, mem[i],
             file_map[i]);
      failed = 1;
    }
  }

  free(mem);
  if (use_userswap) {
    userswap_free((void *)file_map);
  } else {
    free((void *)file_map);
  }

  

  return failed;
}
