CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra -Wpedantic -Werror -std=c11 $(EXTRA_CFLAGS)
LDFLAGS ?= $(EXTRA_LDFLAGS)

LESSON_C := quic_varint.c main.c
BIN      := quic_varint

.PHONY: all clean test

all: $(BIN)

$(BIN): $(LESSON_C) quic_varint.h
	$(CC) $(CFLAGS) -o $@ $(LESSON_C) $(LDFLAGS)

test: all
	@$(MAKE) -C tests

clean:
	rm -f $(BIN) *.o
	@$(MAKE) -C tests clean 2>/dev/null || true
