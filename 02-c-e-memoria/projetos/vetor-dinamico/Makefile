CC ?= cc
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Werror
SANITIZERS ?= -fsanitize=address,undefined
test_vector: vector.c vector.h test_vector.c
	$(CC) $(CFLAGS) $(SANITIZERS) vector.c test_vector.c -o test_vector
test: test_vector
	./test_vector
clean:
	rm -f test_vector
