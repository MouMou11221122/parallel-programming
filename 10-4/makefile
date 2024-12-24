CC = mpicc
SRC = monte_carlo.c
EXECUTABLE = monte_carlo
CSV = output.csv
LOG = time.log
NP_VALUES = 1 2 3 4 5 6 7 8
CFLAGS = -Wall -lm

.PHONY: all clean run

all: $(EXECUTABLE) 

$(EXECUTABLE): $(SRC)
	@$(CC) -o $@ $< $(CFLAGS)

run:
	@cat /dev/null > time.log
	@for np in $(NP_VALUES); do \
	echo "Running with NP=$$np"; \
	mpirun -np $$np ./$(EXECUTABLE) $(CFLAGS); \
	done
clean:
	@rm -rf $(EXECUTABLE) $(CSV) $(LOG) 
