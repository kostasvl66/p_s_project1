CC = gcc
CFLAGS = -Wall -O2 -pthread

PROGRAM_Q1 = barrier_q1
PROGRAM_Q2 = barrier_q2
PROGRAM_Q3 = barrier_q3
ARGS = 1000000 4

$(PROGRAM_Q1): $(PROGRAM_Q1).c
	@$(CC) $(CFLAGS) $< -o $@ 

$(PROGRAM_Q2): $(PROGRAM_Q2).c
	@$(CC) $(CFLAGS) $< -o $@

$(PROGRAM_Q3): $(PROGRAM_Q3).c
	@$(CC) $(CFLAGS) $< -o $@

run-q1: $(PROGRAM_Q1)
	./$(PROGRAM_Q1) $(ARGS)

run-q2: $(PROGRAM_Q2)
	./$(PROGRAM_Q2) $(ARGS)

run-q3: $(PROGRAM_Q3)
	./$(PROGRAM_Q3) $(ARGS)

clean-q1:
	@rm -f $(PROGRAM_Q1)

clean-q2:
	@rm -f $(PROGRAM_Q2)

clean-q3:
	@rm -f $(PROGRAM_Q3)

clean:
	@rm -f $(PROGRAM_Q1) $(PROGRAM_Q2)
