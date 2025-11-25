CC = gcc
CFLAGS = -Wall -O2
ARGS = 10

PROGRAM = arrays

$(PROGRAM): arrays.c
	@$(CC) $(CFLAGS) $< -o $@ -lpthread

run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)

clean:
	@rm -f *.o $(PROGRAM)