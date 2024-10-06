#include <assert.h>
#include <check/check.h>
#include <clock/clock.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// #include "hexdump.c"
#include "psnip_timespec.h"
#include "shad256.h"
#include <aes-accumulator.h>
#include <aes-generator.h>
#include <hexdump.h>

static void fae_print_err_core(FortunaAccumulatorError err, const char *file,
                               long line, const char *func) {
  if (err != FAE_SUCCESS) {
    printf("error FAE %s:%li:%s - %i\n", file, line, func, err);
    exit(EXIT_SUCCESS);
  }
}

#define fae_print_err(err) fae_print_err_core(err, __FILE__, __LINE__, __func__)

int main(int argc, char *argv[]) {
  char seed[32];
  FortunaAccumulatorError err = FAE_SUCCESS;
  memset(seed, 'X', sizeof seed);

  struct FortunaAESAccumulator ctx;
  fortuna_aes_accumulator_init(&ctx);

  for (size_t i = 0; i < 2; ++i) {
    fortuna_aes_accumulator_add_random_event(&ctx, 0, 0, seed, sizeof seed);
  }

  // uint8_t output[33];
  size_t count = 15;

  if (argc > 1) {
    errno = 0;
    char *end;
    count = (size_t)strtoul(argv[1], &end, 10);

    if (errno) {
      perror("Error");
      return EXIT_FAILURE;
    }
  }

  printf("# %s\n", "fortuna-aes");
  printf("type: d\n");
  printf("count: %zi\n", count);
  printf("numbit: %zi\n", sizeof(uint8_t) * CHAR_BIT);

  size_t i = 0;
  uint32_t nums[1000];
  size_t num_of_1000i = count / 1000;
  for (i = 0; i < num_of_1000i; ++i) {
    fortuna_aes_accumulator_random_data(&ctx, nums, sizeof nums);
    fae_print_err(err);
    for (size_t j = 0; j < sizeof nums / sizeof nums[0]; ++j) {
      printf("%10" PRIu32 "\n", nums[j]);
    }
  }

  err = fortuna_aes_accumulator_random_data(&ctx, nums,
                                            sizeof nums - num_of_1000i * 1000);
  fae_print_err(err);
  for (i = 0; i < count - num_of_1000i; ++i) {
    printf("%10" PRIu32 "\n", nums[i]);
  }
  /*
  fortuna_accumulator_random_data(&ctx, output, 33);
  hexDump("hello", output, 33);

  fortuna_accumulator_random_data(&ctx, output, 32);
  hexDump("hello", output, 32);

  fortuna_accumulator_random_data(&ctx, output, 32);
  hexDump("hello", output, 32);
*/
  fortuna_aes_accumulator_destroy(&ctx);
}
