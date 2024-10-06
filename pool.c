#include "pool.h"
#include <shad256.h>

void fortuna_pool_init(struct FortunaPool *ctx) { fortuna_pool_reset(ctx); }

void fortuna_pool_reset(struct FortunaPool *ctx) {
  shad256_init(&ctx->pool_hash_ctx);
  ctx->pool_size = 0;
}

void fortuna_pool_write(struct FortunaPool *ctx, const void *data,
                        size_t datalen) {
  shad256_write(&ctx->pool_hash_ctx, data, datalen);
  ctx->pool_size += datalen;
}

void fortuna_pool_close(struct FortunaPool *ctx, uint8_t *hash) {
  shad256_close(&ctx->pool_hash_ctx, hash);
  ctx->pool_size = 0;
}
