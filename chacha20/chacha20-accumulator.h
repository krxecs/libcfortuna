#ifndef AY_CHACHA20_ACCUMULATOR_H
#define AY_CHACHA20_ACCUMULATOR_H

#include <accumulator.h>
#include <chacha20-generator.h>

struct FortunaChaCha20Accumulator {
  struct FortunaAccumulatorAbstract inner_ctx;

  /** Analogous to `G` in the book. */
  struct FortunaChaCha20Generator generator;
};

void fortuna_cc20_accumulator_init(
    struct FortunaChaCha20Accumulator *accumulator);

void fortuna_cc20_accumulator_add_random_event(
    struct FortunaChaCha20Accumulator *accumulator, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size);

FortunaAccumulatorError fortuna_cc20_accumulator_random_data(
    struct FortunaChaCha20Accumulator *accumulator, void *dest,
    size_t dest_size);

void fortuna_cc20_accumulator_destroy(
    struct FortunaChaCha20Accumulator *accumulator);

#endif /* AY_CHACHA20_ACCUMULATOR_H */
