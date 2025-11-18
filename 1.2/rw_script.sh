#!/bin/bash

gcc rw.c -g -Wall -lpthread -o rw
for ((i=1;i<=$1;i++))
do
    ./rw $2
done
