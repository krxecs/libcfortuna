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
#include "tests.h"
#include "u128.h"
#include <aes-accumulator.h>
#include <aes-generator.h>
#include <munit.h>

static MunitResult test_aes_accumulator_general(const MunitParameter params[],
                                                void *user_data_or_fixture) {
  FortunaAccumulatorError err = FAE_SUCCESS;
  struct FortunaAESAccumulator accumulator;

  fortuna_aes_accumulator_init(&accumulator);

  /* Spread some test data across the pools (source number 42). This would be
   * horribly insecure in a real system. */
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    unsigned char test_data[32];
    memset(test_data, 'X', sizeof test_data);
    fortuna_aes_accumulator_add_random_event(&accumulator, 42, i, test_data,
                                             sizeof test_data);
  }

  /* Add more data. */
  for (size_t i = 0; i < NUM_OF_POOLS; ++i) {
    unsigned char test_data[32];
    memset(test_data, 'X', sizeof test_data);
    fortuna_aes_accumulator_add_random_event(&accumulator, 42, i, test_data,
                                             sizeof test_data);
  }

#if 0
#define ARRSZ_EXPECTED_HASH_CTX0 64
  unsigned char expected_hash_ctx0[ARRSZ_EXPECTED_HASH_CTX0] = {
      0xae, 0xf4, 0x2a, 0x5d, 0xcb, 0xdd, 0xab, 0x67, 0xe8, 0xef, 0xa1,
      0x18, 0xe1, 0xb4, 0x7f, 0xde, 0x5d, 0x69, 0x7f, 0x89, 0xbe, 0xb9,
      0x71, 0xb9, 0x9e, 0x6e, 0x8e, 0x5e, 0x89, 0xfb, 0xf0, 0x64};

  unsigned char actual_hash_ctx0[ARRSZ_EXPECTED_HASH_CTX0];
  accumulator.pools[0].pool_hash_ctx
  munit_assert_memory_equal(ARRSZ_EXPECTED_HASH_CTX0, a, b);
#endif /* 0 */

  unsigned char expected_key0[32];
  memset(expected_key0, '\0', sizeof expected_key0);
  munit_assert_memory_equal(sizeof expected_key0, accumulator.generator.key,
                            expected_key0);

  u128 expected_counter0 = u128_init_from_u64(0, 0);
  munit_assert(u128_compare(expected_counter0, accumulator.generator.counter) ==
               0);

  unsigned char expected_result1[32] = {
      0xb7, 0xb8, 0x6b, 0xd9, 0xa2, 0x7d, 0x96, 0xd7, 0xbb, 0x4a, 0xdd,
      0x1b, 0x6b, 0x10, 0xd1, 0x57, 0x23, 0x50, 0xb1, 0xc6, 0x12, 0x53,
      0xdb, 0x2f, 0x8d, 0xa2, 0x33, 0xbe, 0x72, 0x6d, 0xc1, 0x5f};
  unsigned char actual_result1[32];
  err = fortuna_aes_accumulator_random_data(&accumulator, actual_result1, 32);
  munit_assert_int(err, ==, FAE_SUCCESS);
  // hexDump("ar", actual_result1, 32);
  munit_assert_memory_equal(32, expected_result1, actual_result1);

  unsigned char expected_key1[32] = {
      0xf2, 0x3a, 0xd7, 0x49, 0xf3, 0x30, 0x66, 0xff, 0x53, 0xd3, 0x07,
      0x91, 0x4f, 0xbf, 0x5b, 0x21, 0xda, 0x96, 0x67, 0xc7, 0xe8, 0x6b,
      0xa2, 0x47, 0x65, 0x5c, 0x94, 0x90, 0xe9, 0xd9, 0x4a, 0x7c};
  munit_assert_memory_equal(sizeof expected_key1, expected_key1,
                            accumulator.generator.key);

  u128 expected_counter1 = u128_init_from_u64(0, 5);
  munit_assert_int8(
      u128_compare(expected_counter1, accumulator.generator.counter), ==, 0);

  fortuna_aes_accumulator_destroy(&accumulator);
  return MUNIT_OK;
}

static MunitResult
test_aes_accumulator_use_before_reseed(const MunitParameter params[],
                                       void *user_data_or_fixture) {
  struct FortunaAESAccumulator accumulator;
  fortuna_aes_accumulator_init(&accumulator);

  uint8_t actual_result1;
  FortunaAccumulatorError err = fortuna_aes_accumulator_random_data(
      &accumulator, &actual_result1, sizeof actual_result1);
  munit_assert_int(err, ==, FAE_INTERNAL);

  fortuna_aes_accumulator_destroy(&accumulator);
  return MUNIT_OK;
}

static MunitTest tests[] = {
    {
        (char *)"/general",           /* name */
        test_aes_accumulator_general, /* test */
        NULL,                         /* setup */
        NULL,                         /* tear_down */
        MUNIT_TEST_OPTION_NONE,       /* options */
        NULL                          /* parameters */
    },
    {(char *)"/use-before-reseed", test_aes_accumulator_use_before_reseed, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    /* Mark the end of the array with an entry where the test
     * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

MunitSuite fortuna_accumulator_test_suite = {
    (char *)"/accumulator", /* name */
    tests,                  /* tests */
    NULL,                   /* suites */
    1,                      /* iterations */
    MUNIT_SUITE_OPTION_NONE /* options */
};
