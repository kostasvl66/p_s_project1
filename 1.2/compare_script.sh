#!/bin/bash

#Clearing output files before writing to them
> results.txt
> main.out
> mutex.out
> atomic.out
> rw.out

#Running sample executions of each program
#The number of sample executions is given as input when script is called
gcc main.c -g -Wall -lpthread -o main
for ((i=1;i<=$1;i++))
do
    ./main $2 $3 >> main.out
done

gcc mutex.c -g -Wall -lpthread -o mutex
for ((i=1;i<=$1;i++))
do
    ./mutex $2 $3 >> mutex.out
done

gcc atomic.c -g -Wall -lpthread -o atomic
for ((i=1;i<=$1;i++))
do
    ./atomic $2 $3 >> atomic.out
done

gcc rw.c -g -Wall -lpthread -o rw
for ((i=1;i<=$1;i++))
do
    ./rw $2 $3 >> rw.out
done

gcc -g -Wall compare.c -o compare
./compare $1
