#!/bin/bash

gcc atomic.c -g -Wall -lpthread -o atomic
for ((i=1;i<=$1;i++))
do
    ./atomic $2
done
