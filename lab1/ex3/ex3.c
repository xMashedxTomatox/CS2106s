/*************************************
* Lab 1 Exercise 3
* Name: Tan Chen Xiang
* Student No: A0223962A
* Lab Group: 03
*************************************/

#include <stdio.h>
#include <stdlib.h>

#include "function_pointers.h"
#include "node.h"

// The runner is empty now! Modify it to fulfill the requirements of the
// exercise. You can use ex2.c as a template

// DO NOT initialize the func_list array in this file. All initialization
// logic for func_list should go into function_pointers.c.

// Macros
#define SUM_LIST 0
#define INSERT_AT 1
#define DELETE_AT 2
#define ROTATE_LIST 3
#define REVERSE_LIST 4
#define RESET_LIST 5
#define MAP 6

void run_instruction(FILE * file, list *lst, int instr);
void print_list(list *lst);

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Error: expecting 1 argument, %d found\n", argc - 1);
        exit(1);
    }

    // We read in the file name provided as argument
    char *fname = argv[1];
    
    FILE * file = fopen(fname, "r");

    list *lst = (list *)malloc(sizeof(list));
    lst->head = NULL;

    if (file) {
      int c;
      while(fscanf(file, "%d\n", &c) != EOF) {
        run_instruction(file, lst, c);
      }
      reset_list(lst);
      fclose(file);
    } else {
      printf("Error: File not found");
    }

    free(lst);
    // Update the array of function pointers
    // DO NOT REMOVE THIS CALL
    // (You may leave the function empty if you do not need it)
    update_functions();

    // Rest of code logic here
}


void run_instruction(FILE * file, list *lst, int instr) {
    int index, data, offset;
    switch (instr) {
        case SUM_LIST:
            printf("%ld\n", sum_list(lst));
            break;
        case INSERT_AT:
            fscanf(file, "%d %d", &index, &data);
            insert_node_at(lst, index, data);
            break;
        case DELETE_AT:
            fscanf(file, "%d", &index);
            delete_node_at(lst, index);
            break;
        case ROTATE_LIST:
            fscanf(file, "%d", &offset);
            rotate_list(lst, offset);
            break;
        case REVERSE_LIST:
            reverse_list(lst);
            break;
        case RESET_LIST:
            reset_list(lst);
            break;
        case MAP:
            fscanf(file, "%d", &index);
            map(lst, func_list[index]);
    }
}

// Prints out the whole list in a single line
void print_list(list *lst) {
    if (lst->head == NULL) {
        printf("[ ]\n");
        return;
    }

    printf("[ ");
    node *curr = lst->head;
    do {
        printf("%d ", curr->data);
        curr = curr->next;
    } while (curr != lst->head);
    printf("]\n");
}
