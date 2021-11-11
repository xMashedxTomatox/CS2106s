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
  const size_t memory_size = 8 * 1024 * 1024;

  char msg[50];
  printf("[WARN] THIS TEST WRITES HUGE FILES, TEST ON XCNE. PRESS ENTER TO CONTINUE\n");
  if (fgets(msg, 50, stdin) == 0) {}

  uintptr_t *const mem = malloc(memory_size);
  if (!mem) {
    return 1;
  }

  int files[5];
  uintptr_t *handles[5];

  for (int i = 0; i < 5; i++) {
    char tempfn[] = "/tmp/uswlrdflXXXXXX";
    int tempfile = mkstemp(tempfn);
    if (tempfile == -1) {
      fprintf(stderr, "Failed to make temporary file\n");
      return 1;
    }
    unlink(tempfn);
    files[i] = tempfile;
  }

  for (int j = 0; j < 5; j++) {
    for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
      mem[i] = (uintptr_t)(mem + i) + 10 * j * 1024 * 1024;
    }

    if (write(files[j], mem, memory_size) != memory_size) {
      fprintf(stderr, "Failed to write to temporary file\n");
      return 1;
    }
  }

  _Bool failed = 0;
  long checks = 0;
  for (int j = 0; j < 5; j++) {
    uintptr_t *const file_map = userswap_map(files[j], memory_size);
    if (!file_map || file_map == MAP_FAILED) {
      fprintf(stderr, "Failed to map file\n");
      return 1;
    }
    handles[j] = file_map;
    
    for (size_t i = 0; i < memory_size / sizeof(uintptr_t); ++i) {
      if ((uintptr_t)(mem + i) + 10 * j * 1024 * 1024 != file_map[i]) {
        printf("Failed at entry %zu, expected %" PRIxPTR ", got %" PRIxPTR "\n", i, (uintptr_t)(mem + i) + 10 * j * 1024 * 1024,
              file_map[i]);
        failed = 1;
      } else {
        checks++;
      }
    }
  }
  
  for (int i = 0; i < 5; i++) {
    userswap_free((void *)handles[i]);
  }

  free(mem);
  printf("%ld checks performed\n", checks);
  printf("%s\n", failed == 0 ? "Success" : "Failed");

  return failed;
}
