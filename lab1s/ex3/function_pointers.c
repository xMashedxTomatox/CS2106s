/*************************************
* Lab 1 Exercise 3
* Name: Tan Chen Xiang
* Student No: A0223962A
* Lab Group: 03
*************************************/

#include "functions.h"

// Initialize the func_list array here!
int (*func_list[5])(int) = {&add_one, &add_two, &multiply_five, &square, &cube}; 

// You can also use this function to help you with
// the initialization. This will be called in ex3.c.
void update_functions() {
}
