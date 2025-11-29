CFLAGS = -g -Wall -lpthread
T = 4       #Number of threads used for program execution
R = 1000000 #Number of times a thread function increments the shared variable
S = 5       #Number of sample executions for each program

.SILENT:

test:
	chmod +x compare_script.sh
	./compare_script.sh 5 $(T) $(R)

run-all: run-main run-mutex run-rw run-atomic

all: main mutex rw atomic

run-main: clean
	gcc $(CFLAGS) main.c -o main
	echo "Running main(non-deterministic) implementation"
	./main $(T) $(R)

main:
	gcc $(CFLAGS) main.c -o main
	
run-mutex: clean
	gcc $(CFLAGS) mutex.c -o mutex
	echo "Running mutex implementation"
	./mutex $(T) $(R)

mutex: 
	gcc $(CFLAGS) mutex.c -o mutex

run-rw: clean
	gcc $(CFLAGS) rw.c -o rw
	echo "Running rw implementation"
	./rw $(T) $(R)

rw: 
	gcc $(CFLAGS) rw.c -o rw

run-atomic: clean
	gcc $(CFLAGS) atomic.c -o atomic
	echo "Running Atomic Variable implementation"
	./atomic $(T) $(R)

atomic: 
	gcc $(CFLAGS) atomic.c -o atomic

compare:
	gcc -g -Wall compare.c -o compare

clean: main mutex rw atomic compare
	echo "Cleaning"
	rm main mutex rw atomic compare

clean-out: clean
	echo "Removing output files"
	rm  main.out mutex.out rw.out atomic.out
