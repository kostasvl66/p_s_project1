CFLAGS = -g -Wall -lpthread

.SILENT:

run-main: clean
	echo "Compiling main"
	gcc $( CFLAGS ) main.c -o main
	echo "Running main"
	./main

main:
	echo "Compiling main"
	gcc $( CFLAGS ) main.c -o main

clean: main
	echo "Cleaning"
	rm main
