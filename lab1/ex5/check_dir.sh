#!/bin/bash

echo Current directory: $HOSTNAME
echo "Machine Hardware: "$(uname -s -p)
echo Max User Processes: $(ulimit -u)
echo User Processes: $(ps -u -x| wc -l)
echo User With Most Processes: $(ps -e -o user | uniq -c | sort -n | tail -n 1 | 
  awk '{split($0, arr, " "); print arr[2];}')
echo Memory Free '(%):' $(free | grep 'Mem' | awk '{printf "%.4f", $4/$2 * 100}')
echo Swap Free '(%):' $(free | grep 'Swap' | awk '{printf "%.4f", $4/$2 * 100}')
