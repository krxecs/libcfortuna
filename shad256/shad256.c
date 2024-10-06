#include <assert.h>
#include <sha-256.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// #include "hexdump.h"
#include "shad256.h"

void shad256_init(struct SHAd256 *ctx) {
  sha_256_init(&ctx->sha256_round_1, ctx->hash);
}

void shad256_write(struct SHAd256 *ctx, const void *data, size_t datalen) {
  sha_256_write(&ctx->sha256_round_1, data, datalen);
}

void shad256_close(struct SHAd256 *ctx, uint8_t *hash) {
  sha_256_close(&ctx->sha256_round_1);
  calc_sha_256(hash, ctx->hash, 32);
}

/*
int main(void) {
  struct SHAd256 shad256;
  uint8_t hash[32];
  shad256_init(&shad256);
  shad256_write(&shad256, "\x61\x62\x63", 3);
  shad256_close(&shad256, hash);

  hexDump("Hello", hash, sizeof hash / sizeof *hash);
}*/
