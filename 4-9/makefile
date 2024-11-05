CC = mpicc
SRC = parallel_main.c
EXECUTABLE = a.out
CSV = output.csv
LOG = time.log
NP_VALUES = 1 2 3 4 5 6 7 8
N =100000000
CFLAGS = -Wall -lm -O3

.PHYNY: all clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	@$(CC) -o $@ $< $(CFLAGS)

run:
	@cat /dev/null > time.log
	@for np in $(NP_VALUES); do \
		echo "Running with NP=$$np"; \
		mpirun -np $$np ./$(EXECUTABLE) $(N); \
	done
clean:
	@rm -rf $(EXECUTABLE) $(CSV) $(LOG)
