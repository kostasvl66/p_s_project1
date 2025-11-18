CFLAGS = -g -Wall -lpthread
T = 4
R = 1000000

.SILENT:
run-all: run-main run-mutex run-rw run-atomic

all: main mutex rw atomic

run-main: main
	echo "Running main(non-deterministic) implementation"
	./main $(T) $(R)

main:
	echo "Compiling main(non-deterministic) implementation"
	gcc $(CFLAGS) main.c -o main
	
run-mutex: mutex
	echo "Running mutex implementation"
	./mutex $(T) $(R)

mutex: 
	echo "Compiling mutex implementation"
	gcc $(CFLAGS) mutex.c -o mutex

run-rw: rw
	echo "Running mutex implementation"
	./rw $(T) $(R)

rw: 
	echo "Compiling mutex implementation"
	gcc $(CFLAGS) rw.c -o rw

run-atomic: atomic
	echo "Running Atomic Variable implementation"
	./atomic $(T) $(R)

atomic: 
	echo "Compiling Atomic Variable implementation"
	gcc $(CFLAGS) atomic.c -o atomic

.PHONY: clean
clean:
	rm main mutex rw atomic
