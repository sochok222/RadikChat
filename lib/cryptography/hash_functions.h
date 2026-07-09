#ifndef RADIKCHAT_HASH_FUNCTIONS_H
#define RADIKCHAT_HASH_FUNCTIONS_H
#include <stdint.h>

uint8_t *sha256_hash(uint8_t *input, size_t input_length, size_t *output_length);

#endif //RADIKCHAT_HASH_FUNCTIONS_H