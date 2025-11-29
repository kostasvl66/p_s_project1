CC = gcc
BETTER ?= 0
CFLAGS = -Wall -O2 -DBETTER=$(BETTER) -pthread
ARGS = 10

PROGRAM = arrays

$(PROGRAM): arrays.c
	@$(CC) $(CFLAGS) $< -o $@

run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)

clean:
	@rm -f *.o $(PROGRAM)