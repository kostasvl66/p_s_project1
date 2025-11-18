CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =
LDLIBS = 
ARGS = 10 1

PROGRAM = polynomials
OBJ = polynomials.o

$(PROGRAM): $(OBJ)
	@$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

polynomials.o: polynomials.c
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)

clean:
	@rm -f *.o $(PROGRAM)