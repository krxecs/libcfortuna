#ifndef FORTUNA_CHACHA20_GENERATOR_H
#define FORTUNA_CHACHA20_GENERATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct FortunaChaCha20Generator {
  uint8_t key[32];
  uint8_t nonce[8];
  uint64_t counter;
};

enum FortunaCC20GeneratorError {
  CC20_FGEN_SUCCESS,
  CC20_FGEN_INTERNAL,
  CC20_FGEN_E2BIG,
  CC20_FGEN_NOT_SEEDED,
};
typedef enum FortunaCC20GeneratorError FortunaCC20GeneratorError;

void fortuna_cc20_generator_init(struct FortunaChaCha20Generator *ctx);

bool fortuna_cc20_generator_is_seeded(struct FortunaChaCha20Generator *ctx);

void fortuna_cc20_generator_reseed(struct FortunaChaCha20Generator *ctx,
                                   void *seed, size_t seedlen);

int fortuna_cc20_generator_pseudo_random_data(
    struct FortunaChaCha20Generator *ctx, void *output, size_t outputlen);

void fortuna_cc20_generator_destroy(struct FortunaChaCha20Generator *ctx);

#endif /* FORTUNA_CHACHA20_GENERATOR_H */
