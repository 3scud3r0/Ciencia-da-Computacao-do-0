CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -I.. $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

BIN := test_migration

.PHONY: all clean run

all: run

$(BIN): test_migration.c ../quic_migration.c ../quic_migration.h
	$(CC) $(CFLAGS) -o $@ test_migration.c ../quic_migration.c $(LDFLAGS)

run: $(BIN)
	./$(BIN)
	@echo "All QUIC connection migration tests passed."

clean:
	rm -f $(BIN) *.o
