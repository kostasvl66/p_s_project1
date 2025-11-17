#!/bin/bash

gcc bw.c -g -Wall -lpthread -o bw
for ((i=1;i<=$1;i++))
do
    ./bw $2
done
