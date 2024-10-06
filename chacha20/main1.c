#include <chacha20-accumulator.h>
#include <chacha20-generator.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "../hexdump.c"
#include "../hexdump.h"

int main(int argc, char *argv[]) {
  char *seed1[] = {"abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                   "0123456789"};
  struct FortunaChaCha20Accumulator ctx;
  fortuna_cc20_accumulator_init(&ctx);
  for (size_t i = 0; i < 3; ++i) {
    fortuna_cc20_accumulator_add_random_event(&ctx, 0, 0, seed1[i],
                                              strlen(seed1[i]));
  }

  uint8_t output[128];

  fortuna_cc20_accumulator_random_data(&ctx, output, sizeof output);
  hexDump("World", output, sizeof output);
}
