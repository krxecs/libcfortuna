/* SPDX-License-Identifier: 0BSD */

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hexdump.h"
#include <chacha20-accumulator.h>
#include <chacha20-generator.h>
#include <munit.h>

static MunitResult test_cc20_accumulator_general(const MunitParameter params[],
                                                 void *user_data_or_fixture) {
  FortunaAccumulatorError err = FAE_SUCCESS;
  struct FortunaChaCha20Accumulator accumulator;

  fortuna_cc20_accumulator_init(&accumulator);

  /* Spread some test data across the pools (source number 42). This would be
   * horribly insecure in a real system. */
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    unsigned char test_data[32];
    memset(test_data, 'X', sizeof test_data);
    fortuna_cc20_accumulator_add_random_event(&accumulator, 42, i, test_data,
                                              sizeof test_data);
  }

  /* Add more data. */
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    unsigned char test_data[32];
    memset(test_data, 'X', sizeof test_data);
    fortuna_cc20_accumulator_add_random_event(&accumulator, 42, i, test_data,
                                              sizeof test_data);
  }

  unsigned char expected_key0[32];
  memset(expected_key0, '\0', sizeof expected_key0);
  munit_assert_memory_equal(sizeof expected_key0, accumulator.generator.key,
                            expected_key0);

  munit_assert_uint64(accumulator.generator.counter, ==, 0);

#define arrsz_result1 128
  unsigned char expected_result1[arrsz_result1] = {
      0xd1, 0xe8, 0x3b, 0x60, 0x4e, 0xf3, 0xd7, 0xc4, 0x4d, 0x51, 0x02, 0x8f,
      0x6f, 0x27, 0x20, 0x75, 0x00, 0x4a, 0x36, 0x47, 0x17, 0xee, 0xc4, 0xc4,
      0x2b, 0x25, 0xc5, 0xdf, 0x3f, 0xed, 0x43, 0xaa, 0x52, 0x55, 0x79, 0xc1,
      0x75, 0xda, 0x2b, 0x14, 0x61, 0xe4, 0x09, 0xc0, 0x53, 0x67, 0xfb, 0x7f,
      0xe4, 0x82, 0xcf, 0x76, 0xfc, 0x0f, 0xc1, 0x48, 0x56, 0x9d, 0x78, 0x20,
      0xeb, 0x29, 0xcc, 0xeb, 0x5d, 0x56, 0x4a, 0xa9, 0xb4, 0xcc, 0xc8, 0x36,
      0x0f, 0x0e, 0xb7, 0xdc, 0xad, 0x40, 0xd1, 0xa2, 0xff, 0x82, 0x46, 0xa3,
      0xfb, 0xa1, 0x1b, 0xdf, 0xa6, 0x83, 0xa7, 0x9c, 0x62, 0x17, 0x6f, 0x96,
      0x57, 0xd6, 0xd4, 0x5a, 0x78, 0x38, 0xd0, 0xea, 0xa9, 0xa8, 0x6c, 0x77,
      0x40, 0x33, 0xe0, 0x16, 0x3b, 0x1d, 0xf4, 0x60, 0xdc, 0xb5, 0x28, 0x99,
      0x12, 0xf2, 0x69, 0xb6, 0xdf, 0xca, 0x15, 0x85};
  unsigned char actual_result1[arrsz_result1];
  err = fortuna_cc20_accumulator_random_data(&accumulator, actual_result1,
                                             arrsz_result1);
  munit_assert_int(err, ==, FAE_SUCCESS);
  // hexDump("ar", actual_result1, 32);
  munit_assert_memory_equal(arrsz_result1, expected_result1, actual_result1);

  unsigned char expected_key1[32] = {
      0xdd, 0x73, 0x47, 0xdd, 0x8e, 0x9d, 0x1c, 0xc0, 0xb5, 0x62, 0x2d,
      0xf8, 0xe5, 0xbc, 0xd5, 0x67, 0x13, 0x99, 0x42, 0xef, 0xcc, 0x40,
      0x5b, 0xf9, 0x86, 0x5f, 0xbe, 0x0a, 0x51, 0xe2, 0xee, 0x3a};
  munit_assert_memory_equal(sizeof expected_key1, expected_key1,
                            accumulator.generator.key);

  /* Unlike AES accumulator, counter of generator is 4 instead of 5 as AES
   * Generator needs 2 blocks to generate the key, while ChaCha20 generator
   * needs only half of a block for the same. Since it is impossible to generate
   * half a block, it is rounded up to 1 block and ChaCha20 ends up using only 4
   * blocks. */
  munit_assert_uint64(accumulator.generator.counter, ==, 4);

  fortuna_cc20_accumulator_destroy(&accumulator);
  return MUNIT_OK;
}

static MunitResult
test_cc20_accumulator_use_before_reseed(const MunitParameter params[],
                                        void *user_data_or_fixture) {
  struct FortunaChaCha20Accumulator accumulator;
  fortuna_cc20_accumulator_init(&accumulator);

  uint8_t actual_result1;
  FortunaAccumulatorError err = fortuna_cc20_accumulator_random_data(
      &accumulator, &actual_result1, sizeof actual_result1);
  munit_assert_int(err, ==, FAE_INTERNAL);

  fortuna_cc20_accumulator_destroy(&accumulator);
  return MUNIT_OK;
}

static MunitTest tests[] = {
    {
        (char *)"/general",            /* name */
        test_cc20_accumulator_general, /* test */
        NULL,                          /* setup */
        NULL,                          /* tear_down */
        MUNIT_TEST_OPTION_NONE,        /* options */
        NULL                           /* parameters */
    },
    {(char *)"/use-before-reseed", test_cc20_accumulator_use_before_reseed,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    /* Mark the end of the array with an entry where the test
     * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

static MunitSuite cc20_accumulator_test_suite = {
    (char *)"/chacha20-accumulator", /* name */
    tests,                           /* tests */
    NULL,                            /* suites */
    1,                               /* iterations */
    MUNIT_SUITE_OPTION_NONE          /* options */
};

int main(int argc, char *argv[]) {
  return munit_suite_main(&cc20_accumulator_test_suite, NULL, argc, argv);
}
