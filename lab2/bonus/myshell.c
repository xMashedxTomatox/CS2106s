/**
 * CS2106 AY21/22 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 * Name: Tan Chen Xiang
 * Student No: A0223962A
 * Lab Group: 03
 */

#include "myshell.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct Process {
  pid_t pid;
  bool running;
  bool terminating;
  bool stopped;
  bool waiting;
  int result;
} process_t;

process_t* p_array;
int processIndex = 0;

void ctrl_c() {
 
  for (int i = 0; i < processIndex; i++) {
   if (p_array[i].waiting) { 
      kill(p_array[i].pid, SIGINT);
      p_array[i].terminating = true;
      printf("\n[%d] interrupted\n", p_array[i].pid);
   }
  }
}

void ctrl_z() {

  for (int i = 0; i < processIndex; i++) {
    if (p_array[i].waiting) {
      kill(p_array[i].pid, SIGSTOP);
      p_array[i].stopped = true;
      printf("\n[%d] stopped\n", p_array[i].pid);
      return;
    }
  }
}

void my_init(void) {
  (void) signal(SIGTSTP, ctrl_z);
  (void) signal(SIGINT, ctrl_c);

  p_array = malloc(MAX_PROCESSES * sizeof(process_t));
}

void completeProcess(process_t* p) {
  int status;
  p -> waiting = true;
  waitpid(p -> pid, &status, WUNTRACED);
  p -> waiting = false;
  p -> running = false;
  p -> terminating = false;
  p -> result = WEXITSTATUS(status);
}


bool run_process(size_t num_tokens, char** tokens) {
  if (access(tokens[0], F_OK) != 0) {
    printf("%s not found\n", tokens[0]);
    return false;
  }
  
  bool hasRead = false;
  bool hasWrite = false;
  bool hasError = false;
  int readStream, writeStream, errorStream;

  for (size_t i = 0; i < num_tokens - 1; i++) {
    if (strcmp(tokens[i], "<") == 0) {
      tokens[i] = NULL;
      hasRead = true;
      readStream = open(tokens[i+1], O_RDONLY);
      if (readStream == -1) {
        printf("%s does not exist\n", tokens[i+1]);
        return false;
      }
    }
    else if (strcmp(tokens[i], ">") == 0) {
      tokens[i] = NULL;
      hasWrite = true;
      writeStream = open(tokens[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    } else if (strcmp(tokens[i], "2>") == 0) {
      tokens[i] = NULL;
      hasError = true;
      errorStream = open(tokens[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    }
  }

  bool background = strcmp(tokens[num_tokens-2], "&") == 0;

  int result = fork();

  if (result == 0) {
    signal(SIGTSTP, SIG_IGN);
    if (background)
      tokens[num_tokens-2] = NULL;
    if (hasRead) {
      dup2(readStream, STDIN_FILENO);
      close(readStream);
    }
    if (hasWrite) {
      dup2(writeStream, STDOUT_FILENO);
      close(writeStream);
    }
    if (hasError) {
      dup2(errorStream, STDERR_FILENO);
      close(errorStream);
    }
    execv(tokens[0], tokens);
    return true;
  } else {
    int i = processIndex;
    processIndex++;
    p_array[i].pid = result;
    p_array[i].running = false;
    p_array[i].terminating = false;
    p_array[i].stopped = false;
    p_array[i].waiting = false;

    if (!background) {
      completeProcess(&p_array[i]);
    } else {
      p_array[i].running = true;
      printf("Child[%d] in background\n", result);
    }
    return true;
  }
}

bool my_mini_process_command(size_t num_tokens, char **tokens) {

  if (strcmp(tokens[0], "fg") == 0) {
    for (int i = 0; i < processIndex; i++) {
      if (atoi(tokens[1]) == p_array[i].pid) {
        if (p_array[i].stopped) {
          kill(p_array[i].pid, SIGCONT);
          p_array[i].stopped = false;
          completeProcess(&p_array[i]);
        }
        return false;
      }
    }
    return false;
  }

  if (strcmp(tokens[0], "wait") == 0) {
    for (int i = 0; i < processIndex; i++) {
      if (p_array[i].running && atoi(tokens[1]) == p_array[i].pid) {
        completeProcess(&p_array[i]);
        return false;        
      }
    }
    return false;
  }

  if (strcmp(tokens[0], "terminate") == 0) {
    for (int i = 0; i< processIndex; i++) {
      if (atoi(tokens[1]) == p_array[i].pid && (p_array[i].running || p_array[i].stopped)) {
        if (p_array[i].stopped)
          kill(p_array[i].pid, SIGCONT);

        p_array[i].terminating = true;
        p_array[i].stopped = false;
        kill(p_array[i].pid, SIGTERM);
        return false;
      }
    }
    return false;
  }

  if (strcmp(tokens[0], "info") == 0) {
    for (int i = 0; i < processIndex; i++) {
      printf("[%d] ", p_array[i].pid);
      if (p_array[i].stopped) {
        printf("Stopped\n");
        continue;
      } else if (p_array[i].terminating || p_array[i].running) {
        int status;
        int cpid = waitpid(p_array[i].pid, &status, WNOHANG);
        if(cpid == 0) {
          if (p_array[i].terminating) {
            printf("Terminating\n");
            continue;
          }
          printf("Running\n");
          continue;
        } else {
          p_array[i].running = false;
          p_array[i].terminating = false;
          p_array[i].result = WEXITSTATUS(status);
        }
      } 
      printf("%s %d\n", "Exited", p_array[i].result);      
    }
    return false;
  }

  return run_process(num_tokens, tokens);
}


void my_process_command(size_t num_tokens, char **tokens) {

  bool reset = true;
  char** start;
  size_t count;
  for (size_t i = 0; i < num_tokens - 1; i++) {
    if (reset) {
      count = 1;
      start = &tokens[i];
      reset = false;
    }
    else if (strcmp(tokens[i], "&&") == 0) {
      tokens[i] = NULL;
      if (!my_mini_process_command(count + 1, start))
        return;
      else {
        if (p_array[processIndex-1].running) {
          completeProcess(&p_array[processIndex-1]);
        }
        if (p_array[processIndex-1].result != 0) {
          printf("%s failed\n", start[0]);
          return;
        }
      }
      reset = true;
    } else {
      count++;
    }
  }

  my_mini_process_command(count + 1, start);
}       

void my_quit(void) {
  for (int i = 0; i < processIndex; i++) {
    if (p_array[i].running || p_array[i].stopped) {
      if (p_array[i].stopped)
        kill(p_array[i].pid, SIGCONT);

      kill(p_array[i].pid, SIGTERM);
    }
  }
  printf("Goodbye!\n");
  free(p_array);
}
