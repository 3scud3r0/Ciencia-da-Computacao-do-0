CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -I.. $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

BIN := test_cc

.PHONY: all clean run

all: run

$(BIN): test_cc.c ../quic_cc.c ../quic_cc.h
	$(CC) $(CFLAGS) -o $@ test_cc.c ../quic_cc.c $(LDFLAGS)

run: $(BIN)
	./$(BIN)
	@echo "All QUIC congestion control tests passed."

clean:
	rm -f $(BIN) *.o
