#include "myshell.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>

typedef struct Process {
  pid_t pid;
  bool running;
  int result;
} process_t;

process_t* p_array;
int processIndex = 0;

void my_init(void) {
  p_array = (process_t*)malloc(MAX_PROCESSES * sizeof(process_t));
}

void my_process_command(size_t num_tokens, char **tokens) {

  if (strcmp(tokens[0], "info") == 0) {
    for (int i = 0; i < processIndex; i++) {
      printf("[%d] ", p_array[i].pid);
      if (p_array[i].running) {
        int status;
        int cpid = waitpid(p_array[i].pid, &status, WNOHANG);
        if(cpid == 0) {
          printf("Running\n");
          continue;
        } else {
          p_array[i].running = false;
          p_array[i].result = WEXITSTATUS(status);
        }
      } 
      printf("%s %d\n", "Exited", p_array[i].result);      
    }
    return;
  } 

  bool background = strcmp(tokens[num_tokens-2], "&") == 0;

  int result = fork();
  if (result == 0) {
    if (background)
      tokens[num_tokens-2] = NULL;
    execv(tokens[0], tokens);
    printf("%s not found\n", tokens[0]);
    return;
  } else {
    p_array[processIndex].pid = result;
    if (!background) {
      int status;
      waitpid(p_array[processIndex].pid, &status, 0);
      p_array[processIndex].result = WEXITSTATUS(status);
    } else {
      p_array[processIndex].running = true;
      printf("Child[%d] in background\n", result);
    }
    processIndex++;
  }

}

void my_quit(void) {
  printf("Goodbye!\n");
  free(p_array);
  exit(0);
}
