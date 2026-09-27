CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -I.. $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

BIN := test_packet

.PHONY: all clean run

all: run

$(BIN): test_packet.c ../quic_packet.c ../quic_packet.h
	$(CC) $(CFLAGS) -o $@ test_packet.c ../quic_packet.c $(LDFLAGS)

run: $(BIN)
	./$(BIN)
	@echo "All QUIC packet tests passed."

clean:
	rm -f $(BIN) test_quic *.o
