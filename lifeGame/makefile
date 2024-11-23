CC = mpicc
SRC = lifeGame.c
EXECUTABLE = lifeGame
CSV = output.csv
LOG = time.log
NP_VALUES = 1 2 3 4 5 6 7 8
ITERATION = 30
FREQ = 5
CFLAGS = -Wall -O3

MATRIX_BIN = hugeMatrix.bin
BUILD_MATRIX_SRC = buildHugeMatrix.c
BUILD_MATRIX_BIN = buildHugeMatrix

.PHONY: all clean run

all: $(EXECUTABLE) $(MATRIX_BIN)

$(EXECUTABLE): $(SRC)
	@$(CC) -o $@ $< $(CFLAGS)

$(MATRIX_BIN): $(BUILD_MATRIX_BIN)
	@./$< 45000 500 $@ > /dev/null

$(BUILD_MATRIX_BIN): $(BUILD_MATRIX_SRC)
	@gcc -Wall -o $@ $<

run:
	@cat /dev/null > time.log
	@for np in $(NP_VALUES); do \
	echo "Running with NP=$$np"; \
	mpirun -np $$np ./$(EXECUTABLE) $(MATRIX_BIN) $(N) $(ITERATION) $(FREQ); \
	done
clean:
	@rm -rf $(EXECUTABLE) $(CSV) $(LOG) $(MATRIX_BIN) $(BUILD_MATRIX_BIN)
