#ifndef AY_AES_GENERATOR_WRAPPER_H
#define AY_AES_GENERATOR_WRAPPER_H

#include <accumulator.h>
#include <aes-generator.h>

struct FortunaGeneratorVtable get_aes_generator_vtable(void);

struct FortunaAESAccumulator {
  struct FortunaAccumulatorAbstract inner_ctx;

  /** Analogous to `G` in the book. */
  struct FortunaAESGenerator generator;
};

void fortuna_aes_accumulator_init(struct FortunaAESAccumulator *accumulator);

void fortuna_aes_accumulator_add_random_event(
    struct FortunaAESAccumulator *accumulator, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size);

FortunaAccumulatorError
fortuna_aes_accumulator_random_data(struct FortunaAESAccumulator *accumulator,
                                    void *dest, size_t dest_size);

void fortuna_aes_accumulator_destroy(struct FortunaAESAccumulator *accumulator);

#endif /* AY_AES_GENERATOR_WRAPPER_H */
