CC = mpicc
SRC = matrixMultiply.c
EXECUTABLE = matrixMultiply
CSV = output.csv
LOG = time.log
NP_VALUES = 1 2 3 4 5 6 7 8
CFLAGS = -Wall 

MATRIX_BIN = hugeMatrix.bin
BUILD_MATRIX_SRC = buildHugeMatrix.c
BUILD_MATRIX_BIN = buildHugeMatrix

.PHONY: all clean run

all: $(EXECUTABLE) 

$(EXECUTABLE): $(SRC)
	@$(CC) -o $@ $< $(CFLAGS)

run:
	@cat /dev/null > time.log
	@for np in $(NP_VALUES); do \
	echo "Running with NP=$$np"; \
	mpirun -np $$np ./$(EXECUTABLE) matrix.bin vector.bin; \
	done
clean:
	@rm -rf $(EXECUTABLE) $(CSV) $(LOG) 
