.PHONY: all clean

CC = mpicc
SRC = lifeGame.c
EXEC = $(SRC:.c=)
CFLAGS = -Wall -O3

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(EXEC)
