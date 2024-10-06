#include <accumulator.h>
#include <chacha20-accumulator.h>
#include <chacha20-generator.h>

static void fcg_init(void *ctx) {
  fortuna_cc20_generator_init(
      &((struct FortunaChaCha20Accumulator *)ctx)->generator);
}

static void fcg_reseed(void *ctx, void *seed, size_t seedlen) {
  fortuna_cc20_generator_reseed(
      &((struct FortunaChaCha20Accumulator *)ctx)->generator, seed, seedlen);
}

static int fcg_pseudo_random_data(void *ctx, void *output, size_t outputlen) {
  return fortuna_cc20_generator_pseudo_random_data(
      &((struct FortunaChaCha20Accumulator *)ctx)->generator, output,
      outputlen);
}

static void fcg_destroy(void *ctx) {
  fortuna_cc20_generator_destroy(
      &((struct FortunaChaCha20Accumulator *)ctx)->generator);
}

static struct FortunaGeneratorVtable generator_vtable = {
    .init = &fcg_init,
    .reseed = &fcg_reseed,
    .pseudo_random_data = &fcg_pseudo_random_data,
    .destroy = &fcg_destroy,
};

void fortuna_cc20_accumulator_init(
    struct FortunaChaCha20Accumulator *accumulator) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  fortuna_accumulator_init_general(abstract_ctx, &generator_vtable);
}

void fortuna_cc20_accumulator_add_random_event(
    struct FortunaChaCha20Accumulator *accumulator, uint8_t source_number,
    uint8_t pool_number, const void *data, uint8_t data_size) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  fortuna_accumulator_add_random_event(abstract_ctx, source_number, pool_number,
                                       data, data_size);
}

FortunaAccumulatorError fortuna_cc20_accumulator_random_data(
    struct FortunaChaCha20Accumulator *accumulator, void *dest,
    size_t dest_size) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;

  return fortuna_accumulator_random_data(abstract_ctx, dest, dest_size);
}

void fortuna_cc20_accumulator_destroy(
    struct FortunaChaCha20Accumulator *accumulator) {
  struct FortunaAccumulatorAbstract *abstract_ctx =
      (struct FortunaAccumulatorAbstract *)accumulator;
  fortuna_accumulator_destroy(abstract_ctx);
}
