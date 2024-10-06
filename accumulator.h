#ifndef FORTUNA_ACCUMULATOR_H
#define FORTUNA_ACCUMULATOR_H

#include <clock/clock.h>
#include <stddef.h>
#include <stdint.h>

#include <pool.h>

/**
 * @brief Number of pools in the accumulator.
 */
#define NUM_OF_POOLS 32

struct FortunaGeneratorVtable {
  void (*init)(void *ctx);
  void (*reseed)(void *ctx, void *seed, size_t seedlen);
  int (*pseudo_random_data)(void *ctx, void *output, size_t outputlen);
  void (*destroy)(void *ctx);
};

struct FortunaAccumulatorAbstract {
  struct FortunaGeneratorVtable *vtable;

  /** Analogous to `ReseedCnt` in the book. */
  unsigned int reseed_count;

  struct PsnipClockTimespec last_reseed;

  struct FortunaPool pools[NUM_OF_POOLS];

  // struct SHAd256 pool_hashes[NUM_OF_POOLS];
};

enum FortunaAccumulatorError { FAE_SUCCESS, FAE_INTERNAL, FAE_E2BIG };
typedef enum FortunaAccumulatorError FortunaAccumulatorError;

struct FortunaGeneratorVtable get_aes_generator_vtable(void);

// void fortuna_accumulator_init(struct FortunaAccumulator *accumulator);
void fortuna_accumulator_init_general(
    struct FortunaAccumulatorAbstract *abstract_ctx,
    struct FortunaGeneratorVtable *vtable);

void fortuna_accumulator_add_random_event(
    struct FortunaAccumulatorAbstract *abstract_ctx, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size);

FortunaAccumulatorError
fortuna_accumulator_random_data(struct FortunaAccumulatorAbstract *abstract_ctx,
                                void *dest, size_t dest_size);

void fortuna_accumulator_destroy(
    struct FortunaAccumulatorAbstract *abstract_ctx);

#endif /* FORTUNA_ACCUMULATOR_H */
