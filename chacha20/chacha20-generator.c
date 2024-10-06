#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../hexdump.h"
#include <ay/explicit-memset.h>
#include <chacha20-generator.h>
#include <check/check.h>
#include <monocypher.h>

#define BYTES_PER_BLOCK 64

/** Maximum number of blocks that can be requested by
 * _chacha20_pseudo_random_data function. (2**16) */
#define MAX_BLOCKS_PER_REQUEST (size_t)(1 << 16)

void fortuna_cc20_generator_init(struct FortunaChaCha20Generator *ctx) {
  /* Set key ctx->key & counter ctx->counter to zero. */
  ctx->counter = 0;
  memset(ctx->key, 0, sizeof ctx->key);
  memset(ctx->nonce, 0, sizeof ctx->nonce);
}

bool fortuna_cc20_generator_is_seeded(struct FortunaChaCha20Generator *ctx) {
  return (ctx->counter != 0);
}

void fortuna_cc20_generator_reseed(struct FortunaChaCha20Generator *ctx,
                                   void *seed, size_t seedlen) {
  /* Compute the new key using a hash function. */
  uint8_t new_key[sizeof ctx->key];
  crypto_blake2b_ctx blake2b_ctx;
  crypto_blake2b_general_init(&blake2b_ctx, sizeof ctx->key, NULL, 0);
  crypto_blake2b_update(&blake2b_ctx, ctx->key, sizeof ctx->key);
  crypto_blake2b_update(&blake2b_ctx, seed, seedlen);
  crypto_blake2b_final(&blake2b_ctx, new_key);

  /* Copy new key to ctx->key. */
  memcpy(ctx->key, new_key, sizeof ctx->key);

  /* Clear the temporary key array. */
  (void)ay_explicit_memset(new_key, 0, sizeof new_key);

  /* Increment the counter and mark the generator as seeded. */
  ctx->counter++;
}

static void
_chacha20_pseudo_random_data_no_reseed(struct FortunaChaCha20Generator *ctx,
                                       size_t outputlen, void *output) {
  /* Append neccessary bytes. */
  crypto_chacha20_ctr(output, NULL, outputlen, ctx->key, ctx->nonce,
                      ctx->counter);

  /* Advance the counter to number of full blocks (+ 1 if size of output is not
   * multiple of size of ChaCha20 block) written. */
  ctx->counter += outputlen / BYTES_PER_BLOCK;
  if (outputlen % BYTES_PER_BLOCK != 0) {
    ctx->counter++;
  }
}

int _chacha20_pseudo_random_data(struct FortunaChaCha20Generator *ctx,
                                 void *output, size_t outputlen) {
  /* Check whether generator is seeded or not. */
  if (ctx->counter == 0) {
    return CC20_FGEN_NOT_SEEDED;
  }

  /* Limit output length to reduce statistical deviation from perfectly random
   * outputs. */
  if (outputlen >= MAX_BLOCKS_PER_REQUEST * BYTES_PER_BLOCK) {
    return CC20_FGEN_E2BIG;
  }

  /* Compute the result. */
  _chacha20_pseudo_random_data_no_reseed(ctx, outputlen, output);

  static_assert(sizeof ctx->key == 32,
                "size of key in struct ChaCha20Generator != 32");

  /* Switch to a new key to avoid later compromises of this output. */
  uint8_t tmp_key[sizeof ctx->key];
  _chacha20_pseudo_random_data_no_reseed(ctx, sizeof tmp_key, tmp_key);
  memcpy(ctx->key, tmp_key, sizeof ctx->key);

  /* Clear the temporary key array. */
  ay_explicit_memset(tmp_key, 0, sizeof tmp_key);

  return CC20_FGEN_SUCCESS;
}

int fortuna_cc20_generator_pseudo_random_data(
    struct FortunaChaCha20Generator *ctx, void *output, size_t outputlen) {
  /* psnip_assert_size(outputlen, >=, 0); */
  unsigned char *output0 = (unsigned char *)output;
  size_t num_1mib_blocks = outputlen >> 20;
  size_t remainder = outputlen & ((1 << 20) - 1);

  for (size_t i = 0; i < num_1mib_blocks; ++i) {
    int err = _chacha20_pseudo_random_data(ctx, output0, 1 << 20);
    if (err != CC20_FGEN_SUCCESS) {
      /* _chacha20_pseudo_random_data returning CC20_FGEN_E2BIG is a bug. Abort
       * the program if err == CC20_FGEN_E2BIG and NDEBUG is not defined. */
      psnip_assert_int(err, !=, CC20_FGEN_E2BIG);
      return err == CC20_FGEN_E2BIG ? CC20_FGEN_INTERNAL : err;
    }
    output0 += 1 << 20;
  }

  int err = _chacha20_pseudo_random_data(ctx, output0, remainder);
  if (err == CC20_FGEN_E2BIG) {
    return CC20_FGEN_INTERNAL;
  }

  return err;
}

void fortuna_cc20_generator_destroy(struct FortunaChaCha20Generator *ctx) {
  (void)ay_explicit_memset(ctx, 0, sizeof *ctx);
}
