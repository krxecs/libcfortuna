#include <accumulator.h>
#include <aes-accumulator.h>
#include <aes-generator.h>

static void fg_init(void *ctx) {
  fortuna_aes_generator_init(&((struct FortunaAESAccumulator *)ctx)->generator);
}

static void fg_reseed(void *ctx, void *seed, size_t seedlen) {
  fortuna_aes_generator_reseed(
      &((struct FortunaAESAccumulator *)ctx)->generator, seed, seedlen);
}

static int fg_pseudo_random_data(void *ctx, void *output, size_t outputlen) {
  return fortuna_aes_generator_pseudo_random_data(
      &((struct FortunaAESAccumulator *)ctx)->generator, output, outputlen);
}

static void fg_destroy(void *ctx) {
  fortuna_aes_generator_destroy(
      &((struct FortunaAESAccumulator *)ctx)->generator);
}

static struct FortunaGeneratorVtable generator_vtable = {
    .init = &fg_init,
    .reseed = &fg_reseed,
    .pseudo_random_data = &fg_pseudo_random_data,
    .destroy = &fg_destroy,
};

void fortuna_aes_accumulator_init(struct FortunaAESAccumulator *accumulator) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  fortuna_accumulator_init_general(abstract_ctx, &generator_vtable);
}

void fortuna_aes_accumulator_add_random_event(
    struct FortunaAESAccumulator *accumulator, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  fortuna_accumulator_add_random_event(abstract_ctx, source_number, pool_number,
                                       data, data_size);
}

FortunaAccumulatorError
fortuna_aes_accumulator_random_data(struct FortunaAESAccumulator *accumulator,
                                    void *dest, size_t dest_size) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  return fortuna_accumulator_random_data(abstract_ctx, dest, dest_size);
}

void fortuna_aes_accumulator_destroy(
    struct FortunaAESAccumulator *accumulator) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;
  fortuna_accumulator_destroy(abstract_ctx);
}
