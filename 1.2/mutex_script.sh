#!/bin/bash

gcc mutex.c -g -Wall -lpthread -o mutex
for ((i=1;i<=$1;i++))
do
    ./mutex $2
done
