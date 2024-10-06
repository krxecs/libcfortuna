# SPDX-License-Identifier: 0BSD

CPMGetPackage(Munit.cmake)
munit_cmake_init()

include(DownloadMunitCPM)

add_executable(fortuna_tests main.c aes-generator.test.c aes-accumulator.test.c)
target_link_libraries(fortuna_tests fortuna-aes-accumulator fortuna-aes-generator fortuna-pool munit)

munit_discover_tests(fortuna_tests
#  ITERATIONS 5
#  NO_FORK
#  SHOW_STDERR
#  FATAL_FAILURES
#  SINGLE
#  SEED 0xc5
#  PARAMS a b
#  LOG_VISIBLE info
#  LOG_FATAL error
#  EXTRA_ARGS --color always
#  TEST_PREFIX ay_
#  TEST_SUFFIX _ya
)

add_executable(chacha20-generator_tests chacha20/generator.test.c)
target_link_libraries(chacha20-generator_tests fortuna-chacha20-generator munit)
munit_discover_tests(chacha20-generator_tests)

add_executable(chacha20-accumulator_tests chacha20/accumulator.test.c)
target_link_libraries(chacha20-accumulator_tests fortuna-chacha20-accumulator munit)
munit_discover_tests(chacha20-accumulator_tests)

add_executable(shad256-pool_tests shad256-pool.test.c)
target_link_libraries(shad256-pool_tests fortuna-pool munit)
munit_discover_tests(shad256-pool_tests)
