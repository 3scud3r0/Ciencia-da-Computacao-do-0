CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -I.. $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

BIN := test_varint

.PHONY: all clean run

all: run

$(BIN): test_varint.c ../quic_varint.c ../quic_varint.h
	$(CC) $(CFLAGS) -o $@ test_varint.c ../quic_varint.c $(LDFLAGS)

run: $(BIN)
	./$(BIN)
	@echo "All QUIC varint tests passed."

clean:
	rm -f $(BIN) *.o
