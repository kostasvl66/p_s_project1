CC = gcc
CFLAGS = -Wall -O2
ARGS = 3 1

PROGRAM = polynomials

$(PROGRAM): polynomials.c
	@$(CC) $(CFLAGS) $< -o $@ -lpthread

run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)

clean:
	@rm -f *.o $(PROGRAM)