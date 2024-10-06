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
#include "u128.h"
#include <munit.h>
#include <pool.h>

static MunitResult test_fortuna_pool(const MunitParameter params[],
                                     void *user_data_or_fixture) {
  struct FortunaPool pool;
  fortuna_pool_init(&pool);

  munit_assert_size(pool.pool_size, ==, 0);

  fortuna_pool_write(&pool, "abc", strlen("abc"));
  const unsigned char expected_hash0[FORTUNA_POOL_HASH_SIZE] = {
      0x4f, 0x8b, 0x42, 0xc2, 0x2d, 0xd3, 0x72, 0x9b, 0x51, 0x9b, 0xa6,
      0xf6, 0x8d, 0x2d, 0xa7, 0xcc, 0x5b, 0x2d, 0x60, 0x6d, 0x05, 0xda,
      0xed, 0x5a, 0xd5, 0x12, 0x8c, 0xc0, 0x3e, 0x6c, 0x63, 0x58};
  unsigned char actual_hash0[FORTUNA_POOL_HASH_SIZE];
  munit_assert_size(pool.pool_size, ==, 3);
  fortuna_pool_close(&pool, actual_hash0);
  munit_assert_memory_equal(FORTUNA_POOL_HASH_SIZE, expected_hash0,
                            actual_hash0);

  fortuna_pool_reset(&pool);

  fortuna_pool_init(&pool);
  fortuna_pool_write(&pool, "abc", strlen("abc"));
  const char *data1 = "dbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  fortuna_pool_write(&pool, data1, strlen(data1));
  munit_assert_size(pool.pool_size, ==, 56);

  const unsigned char expected_hash1[FORTUNA_POOL_HASH_SIZE] = {
      0x0c, 0xff, 0xe1, 0x7f, 0x68, 0x95, 0x4d, 0xac, 0x3a, 0x84, 0xfb,
      0x14, 0x58, 0xbd, 0x5e, 0xc9, 0x92, 0x09, 0x44, 0x97, 0x49, 0xb2,
      0xb3, 0x08, 0xb7, 0xcb, 0x55, 0x81, 0x2f, 0x95, 0x63, 0xaf};
  unsigned char actual_hash1[FORTUNA_POOL_HASH_SIZE];
  fortuna_pool_close(&pool, actual_hash1);
  munit_assert_memory_equal(FORTUNA_POOL_HASH_SIZE, expected_hash1,
                            actual_hash1);

  fortuna_pool_reset(&pool);

  munit_assert_size(pool.pool_size, ==, 0);

  const long ten_to_power_6 = 1000 * 1000;
  for (size_t i = 0; i < ten_to_power_6; ++i) {
    fortuna_pool_write(&pool, "a", strlen("a"));
  }
  munit_assert_long((long)pool.pool_size, ==, ten_to_power_6);
  const unsigned char expected_hash2[FORTUNA_POOL_HASH_SIZE] = {
      0x80, 0xd1, 0x18, 0x94, 0x77, 0x56, 0x3e, 0x1b, 0x52, 0x06, 0xb2,
      0x74, 0x9f, 0x1a, 0xfe, 0x48, 0x07, 0xe5, 0x70, 0x5e, 0x8b, 0xd7,
      0x78, 0x87, 0xa6, 0x01, 0x87, 0xa7, 0x12, 0x15, 0x66, 0x88};
  unsigned char actual_hash2[FORTUNA_POOL_HASH_SIZE];
  fortuna_pool_close(&pool, actual_hash2);
  munit_assert_memory_equal(FORTUNA_POOL_HASH_SIZE, expected_hash2,
                            actual_hash2);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    {(char *)"/general", test_fortuna_pool, NULL, NULL, MUNIT_TEST_OPTION_NONE,
     NULL},
    /* Mark the end of the array with an entry where the test
     * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

MunitSuite shad256_pool_test_suite = {
    (char *)"/shad256-pool", /* name */
    tests,                   /* tests */
    NULL,                    /* suites */
    1,                       /* iterations */
    MUNIT_SUITE_OPTION_NONE  /* options */
};

int main(int argc, char *argv[]) {
  return munit_suite_main(&shad256_pool_test_suite, NULL, argc, argv);
}
