#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
// #include <endian/endian.h>
#include <aes.h>
#include <ay/explicit-memset.h>
#include <check/check.h>

#include "hexdump.h"
#include "u128.h"
#include <aes-generator.h>
#include <shad256.h>

#define BYTES_PER_BLOCK 16

void fortuna_aes_generator_init(struct FortunaAESGenerator *ctx) {
  /* Set key ctx->key & counter ctx->counter to zero. */
  ctx->is_seeded = false;
  ctx->counter = u128_init_from_u64(0, 0);
  memset(ctx->key, 0, sizeof ctx->key);
}

void fortuna_aes_generator_reseed(struct FortunaAESGenerator *ctx, void *seed,
                                  size_t seedlen) {
  /* Mark generator as seeded. */
  if (!ctx->is_seeded) {
    ctx->is_seeded = true;
  }

  /* Compute the new key using a hash function. */
  struct SHAd256 shad256;
  shad256_init(&shad256);
  shad256_write(&shad256, ctx->key, sizeof ctx->key);
  shad256_write(&shad256, seed, seedlen);
  shad256_close(&shad256, ctx->key);

  /* Increment the counter */
  u128_increment(&ctx->counter);
}

int fortuna_aes_generator_generate_blocks(struct FortunaAESGenerator *ctx,
                                          void *output_addr,
                                          size_t num_of_blocks) {
  if (!ctx->is_seeded) {
    return FGEN_NOT_SEEDED;
  }

  psnip_assert_int8(u128_compare(ctx->counter, u128_init_from_u64(0, 0)), !=,
                    0);

  uint8_t *output = output_addr;

  /* Append neccessary blocks. */
  for (size_t i = 0; i < num_of_blocks; ++i) {
    // hexDump("1", &ctx->counter, 16);
    struct AES_ctx aes_ctx;
    memset(output, 0, 16);
    AES_init_ctx_iv(&aes_ctx, ctx->key, (uint8_t *)&ctx->counter);
    AES_CTR_xcrypt_buffer(&aes_ctx, output, 16);

    output += 16;
    u128_increment(&ctx->counter);
  }

  return FGEN_SUCCESS;
}

/* 2**16 */
#define MAX_BLOCKS_PER_REQUEST (size_t)(1 << 16)

int _pseudo_random_data(struct FortunaAESGenerator *ctx, void *output,
                        size_t outputlen) {
  unsigned char *output0 = (unsigned char *)output;
  /* Limit output length to reduce statistical deviation from perfectly random
   * outputs. */
  if (outputlen >= MAX_BLOCKS_PER_REQUEST * BYTES_PER_BLOCK) {
    return FGEN_E2BIG;
  }

  /* Compute the result. */
  size_t num_of_blocks = outputlen / AES_BLOCKLEN;
  fortuna_aes_generator_generate_blocks(ctx, output0, num_of_blocks);
  output0 += num_of_blocks * AES_BLOCKLEN;

  // Add data for remaining output size
  size_t remaining_size = outputlen - num_of_blocks * AES_BLOCKLEN;
  if (remaining_size > 0) {
    uint8_t last_block[AES_BLOCKLEN];
    FortunaGeneratorError err =
        fortuna_aes_generator_generate_blocks(ctx, last_block, 1);
    if (err != FGEN_SUCCESS) {
      return err;
    }
    memcpy(output0, last_block, remaining_size);
  }

  /* Switch to a new key to avoid later compromises of this output. */
  uint8_t tmp_key[32];
  fortuna_aes_generator_generate_blocks(ctx, tmp_key, 2);
  memcpy(ctx->key, tmp_key, 32);

  return FGEN_SUCCESS;
}

int fortuna_aes_generator_pseudo_random_data(struct FortunaAESGenerator *ctx,
                                             void *output, size_t outputlen) {
  /* psnip_assert_size(outputlen, >=, 0); */
  unsigned char *output0 = (unsigned char *)output;
  size_t num_1mib_blocks = outputlen >> 20;
  size_t remainder = outputlen & ((1 << 20) - 1);

  for (size_t i = 0; i < num_1mib_blocks; ++i) {
    FortunaGeneratorError err = _pseudo_random_data(ctx, output0, 1 << 20);
    switch (err) {
    case FGEN_E2BIG:
      return FGEN_INTERNAL;
      break;

    default:
      return err;
    }
    output0 += 1 << 20;
  }

  FortunaGeneratorError err = _pseudo_random_data(ctx, output0, remainder);
  switch (err) {
  case FGEN_E2BIG:
    return FGEN_INTERNAL;
    break;

  default:
    return err;
  }

  return FGEN_SUCCESS;
}

void fortuna_aes_generator_destroy(struct FortunaAESGenerator *ctx) {
  (void)ay_explicit_memset(ctx, 0, sizeof *ctx);
}
