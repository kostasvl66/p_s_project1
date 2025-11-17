#!/bin/bash

gcc main.c -g -Wall -lpthread -o main
for ((i=1;i<=$1;i++))
do
    ./main $2
done
