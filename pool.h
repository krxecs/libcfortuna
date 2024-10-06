#ifndef AY_POOL_H
#define AY_POOL_H

#include <shad256.h>
#include <stddef.h>

#define FORTUNA_POOL_HASH_SIZE SHAD256_HASH_SIZE

struct FortunaPool {
  size_t pool_size;
  struct SHAd256 pool_hash_ctx;
};

void fortuna_pool_init(struct FortunaPool *ctx);
void fortuna_pool_reset(struct FortunaPool *ctx);
void fortuna_pool_write(struct FortunaPool *ctx, const void *data,
                        size_t datalen);
void fortuna_pool_close(struct FortunaPool *ctx, uint8_t *hash);

#endif /* AY_POOL_H */
