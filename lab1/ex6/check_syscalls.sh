#!/bin/bash

####################
# Lab 1 Exercise 6
# Name: Tan Chen Xiang
# Student No: A0223962A
# Lab Group: 03 
####################

echo "Printing system call report"

# Compile file
gcc -std=c99 pid_checker.c -o ex6

# Use strace to get report
strace -c ./ex6
