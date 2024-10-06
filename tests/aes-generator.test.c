/* SPDX-License-Identifier: 0BSD */

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "tests.h"
#include <aes-generator.h>
#include <munit.h>

static MunitResult test_generator(const MunitParameter params[],
                                  void *user_data_or_fixture) {
  FortunaGeneratorError err;
  struct FortunaAESGenerator generator;
  fortuna_aes_generator_init(&generator);

  /* Seed the generator, which should set the key and increment the counter */
  unsigned char seed1[] = {'H', 'e', 'l', 'l', 'o'};
  fortuna_aes_generator_reseed(&generator, seed1, sizeof seed1 / sizeof *seed1);

  const unsigned char expected_result1[] = {
      0x0e, 0xa6, 0x91, 0x9d, 0x43, 0x61, 0x55, 0x13, 0x64, 0x24, 0x2a,
      0x4b, 0xa8, 0x90, 0xf8, 0xf0, 0x73, 0x67, 0x6e, 0x82, 0xcf, 0x1a,
      0x52, 0xbb, 0x88, 0x0f, 0x7e, 0x49, 0x66, 0x48, 0xb5, 0x65};
  munit_assert_memory_equal(sizeof generator.key / sizeof *generator.key,
                            expected_result1, generator.key);

  /* Read 2 full blocks from the generator */
#define arrsz_expected_result2 32
  const unsigned char expected_result2[arrsz_expected_result2] = {
      /* counter = 1 */
      0x7c,
      0xbe,
      0x2c,
      0x17,
      0x68,
      0x4a,
      0xc2,
      0x23,
      0xd0,
      0x89,
      0x69,
      0xee,
      0x8b,
      0x56,
      0x56,
      0x16,

      /* counter = 2 */
      0x71,
      0x76,
      0x61,
      0xc0,
      0xd2,
      0xf4,
      0x75,
      0x8b,
      0xd6,
      0xba,
      0x14,
      0x0b,
      0xf3,
      0x79,
      0x1a,
      0xbd,
  };
  unsigned char actual_result2[32];

  err = fortuna_aes_generator_pseudo_random_data(&generator, actual_result2,
                                                 arrsz_expected_result2);
  munit_assert_int(err, ==, FGEN_SUCCESS);

  // hexDump("a", actual_result2, arrsz_expected_result2);
  munit_assert_memory_equal(arrsz_expected_result2, expected_result2,
                            actual_result2);

#undef arrsz_expected_result2

  /* Meanwhile, the generator will have re-keyed itself and incremented its
   * counter */
#define arrsz_expected_result3 32
  const unsigned char expected_result3[arrsz_expected_result3] = {
      /* counter = 3 */
      0x33,
      0xa1,
      0xbb,
      0x21,
      0x98,
      0x78,
      0x59,
      0xca,
      0xf2,
      0xbb,
      0xfc,
      0x56,
      0x15,
      0xbe,
      0xf5,
      0x6d,

      /* counter = 4 */
      0xe6,
      0xb7,
      0x1f,
      0xf9,
      0xf3,
      0x71,
      0x12,
      0xd0,
      0xc1,
      0x93,
      0xa1,
      0x35,
      0x16,
      0x08,
      0x62,
      0xb7,
  };
  munit_assert_memory_equal(arrsz_expected_result3, expected_result3,
                            generator.key);

#undef arrsz_expected_result3

  /* Read another 2 blocks from the generator */
#define arrsz_expected_result4 32
  const unsigned char expected_result4[arrsz_expected_result4] = {
      /* counter = 5 */
      0xfd,
      0x66,
      0x48,
      0xba,
      0x30,
      0x86,
      0xe9,
      0x19,
      0xce,
      0xe3,
      0x49,
      0x04,
      0xef,
      0x09,
      0xa7,
      0xff,

      /* counter = 6 */
      0x02,
      0x1f,
      0x77,
      0x58,
      0x05,
      0x58,
      0xb8,
      0xc3,
      0xe9,
      0x24,
      0x82,
      0x75,
      0xf2,
      0x30,
      0x42,
      0xbf,
  };
  unsigned char actual_result4[arrsz_expected_result4];
  err = fortuna_aes_generator_pseudo_random_data(&generator, actual_result4,
                                                 arrsz_expected_result4);
  munit_assert_int(err, ==, FGEN_SUCCESS);
  munit_assert_memory_equal(arrsz_expected_result4, expected_result4,
                            actual_result4);

#undef arrsz_expected_result4

  fortuna_aes_generator_destroy(&generator);
  return MUNIT_OK;
}

static MunitResult
test_generator_use_before_reseed(const MunitParameter params[],
                                 void *user_data_or_fixture) {
  struct FortunaAESGenerator generator;
  fortuna_aes_generator_init(&generator);

  uint8_t actual_result1;
  FortunaGeneratorError err = fortuna_aes_generator_pseudo_random_data(
      &generator, &actual_result1, sizeof actual_result1);
  munit_assert_int(err, ==, FGEN_NOT_SEEDED);

  fortuna_aes_generator_destroy(&generator);
  return MUNIT_OK;
}
static MunitTest tests[] = {
    {
        (char *)"/generator",   /* name */
        test_generator,         /* test */
        NULL,                   /* setup */
        NULL,                   /* tear_down */
        MUNIT_TEST_OPTION_NONE, /* options */
        NULL                    /* parameters */
    },
    {(char *)"/use-before-reseed", test_generator_use_before_reseed, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    /* Mark the end of the array with an entry where the test
     * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

MunitSuite fortuna_generator_test_suite = {
    (char *)"/generator",   /* name */
    tests,                  /* tests */
    NULL,                   /* suites */
    1,                      /* iterations */
    MUNIT_SUITE_OPTION_NONE /* options */
};
