CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -I.. $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

VARINT_DIR := ../../35-quic-overview-rfc-9000
BIN        := test_stream

.PHONY: all clean run

all: run

$(BIN): test_stream.c ../quic_stream.c ../quic_stream.h $(VARINT_DIR)/quic_varint.c $(VARINT_DIR)/quic_varint.h
	$(CC) $(CFLAGS) -o $@ test_stream.c ../quic_stream.c $(VARINT_DIR)/quic_varint.c $(LDFLAGS)

run: $(BIN)
	./$(BIN)
	@echo "All QUIC stream tests passed."

clean:
	rm -f $(BIN) *.o
