/*************************************
 * Lab 1 Exercise 3
 * Name: Tan Chen Xiang
 * Student No: A0223962A
 * Lab Group: 03
 *************************************/

#include "node.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit (although you do not need to)

// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {
  if (lst == NULL)
    return;

  node * n = (node*) malloc(sizeof(node));
  n -> data = data;

  if (lst -> head == NULL) {
    lst -> head = n;
    n -> next = n;
  } else {
    node * tempHead = lst -> head;
    node * curr = tempHead;
    if (index == 0) {
      n -> next = tempHead;
      lst -> head = n;
      while (curr -> next != tempHead) {
        curr = curr -> next;
      }
      curr -> next = lst -> head;
    } else {
      for (int i = 1; i < index; i++) {
        curr = curr -> next;    
      }
      n -> next = curr -> next;
      curr -> next = n;
    }
  } 
}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.
void delete_node_at(list *lst, int index) {
  if (lst == NULL || lst -> head == NULL) 
    return;

  if (index == 0) {
    node * ele0 = lst -> head;
    if (ele0 -> next == ele0) {
      lst -> head = NULL;
    } else {
      lst -> head = ele0 -> next;
      node * curr = lst -> head;
      while(curr -> next != ele0) {
        curr = curr -> next;
      }
      curr -> next = lst -> head;
    }
    free(ele0);
    ele0 = NULL;

  } else {
    node * curr = lst -> head;
    for(int i = 1; i < index; i++) {
      curr = curr -> next;
    }
    node * eleX = curr -> next;
    curr -> next = eleX -> next;
    free(eleX);
    eleX = NULL;
  }
}

// Rotates list by the given offset.
// Note: offset is guarenteed to be non-negative.
void rotate_list(list *lst, int offset) {
  if (lst == NULL || lst -> head == NULL) 
    return;

  node * curr = lst -> head;
  for (int i = 0; i < offset; i++) {
    curr = curr -> next;
  }
  lst -> head = curr;
}

// Reverses the list, with the original "tail" node
// becoming the new head node.
void reverse_list(list *lst) {
  if (lst == NULL || lst -> head == NULL) 
    return;

  node * tempFirst = lst -> head;
  node * curr = tempFirst -> next;
  node * prev = tempFirst;
  while(curr != tempFirst){ 
    node * tempNext = curr -> next;
    curr -> next = prev;
    prev = curr;
    curr = tempNext;
  }
  tempFirst -> next = prev;
  lst -> head = prev;
}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {
  if (lst == NULL || lst -> head == NULL)
    return;
  
  node * tempHead = lst -> head;
  node * curr = lst -> head -> next;

  while (curr != tempHead) {
    node * temp = curr -> next;
    free(curr);
    curr = temp;
  }

  free(tempHead);
  lst -> head = NULL;
}

// Traverses list and applies func on data values of
// all elements in the list.
void map(list *lst, int (*func)(int)) {
  if (lst == NULL || lst -> head == NULL)
    return;
  node * tempHead = lst -> head;
  node * curr = lst -> head;
  do {
    int temp = curr -> data;
    curr -> data = (*func)(temp);
    curr = curr -> next;
  } while (curr != tempHead);
}

// Traverses list and returns the sum of the data values
// of every node in the list.
long sum_list(list *lst) {
  if (lst == NULL || lst -> head == NULL) 
    return 0;
  
  long final = 0;

  node * tempHead = lst -> head;
  node * curr = lst -> head;

  do {
    final += curr -> data;
    curr = curr -> next;
  } while (curr != tempHead);

  return final;
}
