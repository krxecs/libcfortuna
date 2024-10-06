#include <assert.h>
#include <check/check.h>
#include <clock/clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hexdump.h"
#include "psnip_timespec.h"
#include <accumulator.h>
// #include "../hexdump.c"
#include <ay/explicit-memset.h>
#include <pool.h>

/**
 * @brief An estimate of how many bytes we must append to pool 0 before it will
 * contain 128 bits of entropy (with respect to an attack).
 *
 * We reseed the generator only after pool 0 contains `MIN_POOL_SIZE` bytes.
 * Note that unlike with some other PRNGs, Fortuna's security does not rely on
 * the accuracy of this estimate - we can accord to be optimistic here.
 */
#define MIN_POOL_SIZE 64

/**
 * @brief Minimum interval between 2 consecutive reseeds, in seconds.
 *
 * If an attacker can predict some (but not all) of our entropy sources, the
 * `MIN_POOL_SIZE` check may not be sufficient to prevent a successful state
 * compromise extension attack.  To resist this attack, Fortuna spreads the
 * input across 32 pools, which are then consumed (to reseed the output
 * generator) with exponentially decreasing frequency.
 *
 * In order to prevent an attacker from gaining knowledge of all 32 pools
 * before we have a chance to fill them with enough information that the
 * attacker cannot predict, we impose a rate limit of 10 reseeds/second (one
 * per 100 ms).  This ensures that a hypothetical 33rd pool would only be
 * needed after a minimum of 13 years of sustained attack.
 */
const struct PsnipClockTimespec reseed_interval = {.seconds = 0,
                                                   .nanoseconds = 100000000};

void fortuna_accumulator_init_general(
    struct FortunaAccumulatorAbstract *abstract_ctx,
    struct FortunaGeneratorVtable *vtable) {
  abstract_ctx->vtable = vtable;
  abstract_ctx->vtable->init(abstract_ctx);

  abstract_ctx->reseed_count = 0;
  abstract_ctx->last_reseed.seconds = 0;
  abstract_ctx->last_reseed.nanoseconds = 0;

  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    fortuna_pool_init(&abstract_ctx->pools[i]);
  }
}

static void acc__reseed(struct FortunaAccumulatorAbstract *abstract_ctx,
                        struct PsnipClockTimespec current_ts) {
  abstract_ctx->last_reseed = current_ts;
  abstract_ctx->reseed_count++;

  uint8_t seed[NUM_OF_POOLS * FORTUNA_POOL_HASH_SIZE];
  size_t seedsz = 0;
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    if (abstract_ctx->reseed_count % (1 << i) == 0) {
      fortuna_pool_close(&abstract_ctx->pools[i], seed + seedsz);
      fortuna_pool_init(&abstract_ctx->pools[i]);
      seedsz += FORTUNA_POOL_HASH_SIZE;
    }
  }

  abstract_ctx->vtable->reseed(abstract_ctx, seed, seedsz);
}

void fortuna_accumulator_add_random_event(
    struct FortunaAccumulatorAbstract *abstract_ctx, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size) {
  psnip_assert_uint8(data_size, <=, 32);
  psnip_assert_uint8(source_number, <=, 255);
  psnip_assert_uint8(pool_number, <=, 31);

  fortuna_pool_write(&abstract_ctx->pools[pool_number], &source_number,
                     sizeof source_number);
  fortuna_pool_write(&abstract_ctx->pools[pool_number], &data_size,
                     sizeof data_size);
  fortuna_pool_write(&abstract_ctx->pools[pool_number], data, data_size);
}

FortunaAccumulatorError
fortuna_accumulator_random_data(struct FortunaAccumulatorAbstract *abstract_ctx,
                                void *dest, size_t dest_size) {
  struct PsnipClockTimespec current_ts = {0};
  struct PsnipClockTimespec result_ts = {.nanoseconds = 0, .seconds = 0};
  psnip_clock_monotonic_get_time(&current_ts);
  timespec_add(abstract_ctx->last_reseed, reseed_interval, &result_ts);

  if (timespec_cmp(current_ts, result_ts) < 0 &&
      abstract_ctx->pools[0].pool_size >= MIN_POOL_SIZE) {
    /* printf("%lu %li\n", result_ts.seconds, result_ts.nanoseconds); */
    acc__reseed(abstract_ctx, current_ts);
  }

  int err =
      abstract_ctx->vtable->pseudo_random_data(abstract_ctx, dest, dest_size);
  if (err != 0) {
    return FAE_INTERNAL;
  }

  return FAE_SUCCESS;
}

void fortuna_accumulator_destroy(
    struct FortunaAccumulatorAbstract *abstract_ctx) {
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    fortuna_pool_reset(&abstract_ctx->pools[i]);
  }
  abstract_ctx->vtable->destroy(abstract_ctx);
  (void)ay_explicit_memset(abstract_ctx, 0, sizeof *abstract_ctx);
}
