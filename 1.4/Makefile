CFLAGS = -g -Wall -lpthread 

A = 100                  #ARRAY_SIZE
TR = 250                #TRANSACTION_NUM
R = 50                   #READ_PERCENTAGE
L = MUTEX_COARSEGRAINED  #LOCK_TYPE
T = 4                    #THREAD_NUM


.SILENT:

run-main: clean
	gcc -g -Wall main.c -o main -lpthread -lm
	echo "Running main"
	./main $(A) $(TR) $(R) $(L) $ $(T)

main:
	echo "Compiling main"
	gcc -g -Wall main.c -o main -lpthread -lm

clean: main
	echo "Cleaning"
	rm main
