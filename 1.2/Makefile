CFLAGS = -g -Wall -lpthread
T = 4
R = 1000000

.SILENT:

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
	echo "Running erw implementation"
	./rw $(T) $(R)

rw: 
	gcc $(CFLAGS) rw.c -o rw

run-atomic: clean
	gcc $(CFLAGS) atomic.c -o atomic
	echo "Running Atomic Variable implementation"
	./atomic $(T) $(R)

atomic: 
	gcc $(CFLAGS) atomic.c -o atomic

clean: main mutex rw atomic
	echo "Cleaning"
	rm main mutex rw atomic
