#!/bin/bash

####################
# Lab 1 Exercise 5
# Name: Tan Chen Xiang
# Student No: A0223962A
# Lab Group: 03
####################


# Fill the below up
hostname=$HOSTNAME
machine_hardware=$(uname -s -p)
max_user_process_count=$(ulimit -u)
user_process_count=$(ps -u -x| wc -l)
user_with_most_processes=$(ps -e -o user | uniq -c | sort -n | tail -n 1 | 
  awk '{split($0, arr, " "); print arr[2];}')
mem_free_percentage=$(free | grep 'Mem' | awk '{printf "%.4f", $4/$2 * 100}')
swap_free_percentage=$(free | grep 'Swap' | awk '{printf "%.4f", $4/$2 * 100}')

echo "Hostname: $hostname"
echo "Machine Hardware: $machine_hardware"
echo "Max User Processes: $max_user_process_count"
echo "User Processes: $user_process_count"
echo "User With Most Processes: $user_with_most_processes"
echo "Memory Free (%): $mem_free_percentage"
echo "Swap Free (%): $swap_free_percentage"
