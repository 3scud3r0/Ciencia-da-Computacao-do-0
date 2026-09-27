#ifndef CC0_VECTOR_H
#define CC0_VECTOR_H
#include <stddef.h>
typedef struct { int *data; size_t size; size_t capacity; } IntVector;
int vector_init(IntVector *vector);
int vector_push(IntVector *vector, int value);
int vector_get(const IntVector *vector, size_t index, int *out_value);
int vector_set(IntVector *vector, size_t index, int value);
int vector_pop(IntVector *vector, int *out_value);
void vector_free(IntVector *vector);
#endif
