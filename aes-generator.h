#ifndef FORTUNA_GENERATOR_H
#define FORTUNA_GENERATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "u128.h"

struct FortunaAESGenerator {
  bool is_seeded;
  u128 counter;
  uint8_t key[32];
};

enum FortunaGeneratorError {
  FGEN_SUCCESS,
  FGEN_INTERNAL,
  FGEN_E2BIG,
  FGEN_NOT_SEEDED,
};
typedef enum FortunaGeneratorError FortunaGeneratorError;

void fortuna_aes_generator_init(struct FortunaAESGenerator *ctx);

void fortuna_aes_generator_reseed(struct FortunaAESGenerator *ctx, void *seed,
                                  size_t seedlen);

int fortuna_aes_generator_generate_blocks(struct FortunaAESGenerator *ctx,
                                          void *output_addr,
                                          size_t num_of_blocks);

int _pseudo_random_data(struct FortunaAESGenerator *ctx, void *output,
                        size_t outputlen);

int fortuna_aes_generator_pseudo_random_data(struct FortunaAESGenerator *ctx,
                                             void *output, size_t outputlen);

void fortuna_aes_generator_destroy(struct FortunaAESGenerator *ctx);

#endif /* FORTUNA_GENERATOR_H */
