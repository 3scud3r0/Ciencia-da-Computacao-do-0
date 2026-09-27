CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

VARINT_DIR := ../35-quic-overview-rfc-9000
BIN        := quic_stream

.PHONY: all clean test

all: $(BIN)

$(BIN): main.c quic_stream.c quic_stream.h $(VARINT_DIR)/quic_varint.c $(VARINT_DIR)/quic_varint.h
	$(CC) $(CFLAGS) -o $@ main.c quic_stream.c $(VARINT_DIR)/quic_varint.c $(LDFLAGS)

test: all
	@$(MAKE) -C tests

clean:
	rm -f $(BIN) *.o
	@$(MAKE) -C tests clean 2>/dev/null || true
