#ifndef SHAD256_H
#define SHAD256_H

#include <sha-256.h>
#include <stddef.h>
#include <stdint.h>

#define SHAD256_HASH_SIZE 32

struct SHAd256 {
  struct Sha_256 sha256_round_1;
  uint8_t hash[32];
};

void shad256_init(struct SHAd256 *ctx);
void shad256_write(struct SHAd256 *ctx, const void *data, size_t datalen);
void shad256_close(struct SHAd256 *ctx, uint8_t *hash);

#endif /* SHAD256_H */
