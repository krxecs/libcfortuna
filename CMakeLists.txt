cmake_policy(SET CMP0091 NEW)

cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)

project(fortuna-rng LANGUAGES C)

include(cmake/DefaultConfig.cmake)

####################
# Section: Options #
####################

include(CMakeDependentOption)
include(CTest)

cmake_dependent_option(${PROJECT_NAME}_BUILD_TESTING
  "Force building tests for ${PROJECT_NAME}."
  ON
  "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME"
  OFF
)

include(CPM)

CPMUsePackageLock(package-lock.cmake)

include(FetchContent)

CPMGetPackage(monocypher)

if (monocypher_ADDED)
  add_library(monocypher STATIC "${monocypher_SOURCE_DIR}/src/monocypher.c")
  target_include_directories(monocypher SYSTEM PUBLIC INCLUDE "${monocypher_SOURCE_DIR}/src")
endif ()

CPMAddPackage(
  NAME amosnier-sha-2
  GITHUB_REPOSITORY amosnier/sha-2
  GIT_TAG origin/master
  DOWNLOAD_ONLY YES
)

if (amosnier-sha-2_ADDED)
  add_library(amosnier-sha-2 STATIC "${amosnier-sha-2_SOURCE_DIR}/sha-256.c")
  target_include_directories(amosnier-sha-2 SYSTEM PUBLIC INCLUDE "${amosnier-sha-2_SOURCE_DIR}")
endif ()

#FetchContent_Declare(psnip
#	GIT_REPOSITORY https://github.com/nemequ/portable-snippets
#	CONFIGURE_COMMAND ""
#	BUILD_COMMAND ""
#  GIT_SHALLOW TRUE
#)
#FetchContent_Populate(psnip)
CPMAddPackage(
  NAME psnip
  GITHUB_REPOSITORY nemequ/portable-snippets
  GIT_TAG origin/master
  DOWNLOAD_ONLY YES
)

if (psnip_ADDED)
  add_library(psnip-endian INTERFACE)
  target_sources(psnip-endian INTERFACE "${psnip_SOURCE_DIR}/endian/endian.h")
  target_include_directories(psnip-endian SYSTEM INTERFACE "${psnip_SOURCE_DIR}")

  add_library(psnip-check INTERFACE)
  target_sources(psnip-check INTERFACE "${psnip_SOURCE_DIR}/check/check.h")
  target_include_directories(psnip-check SYSTEM INTERFACE "${psnip_SOURCE_DIR}")

  add_library(psnip-clock INTERFACE)
  target_sources(psnip-clock INTERFACE "${psnip_SOURCE_DIR}/clock/clock.h")
  target_include_directories(psnip-clock SYSTEM INTERFACE "${psnip_SOURCE_DIR}")

  add_library(psnip-builtin INTERFACE)
  target_sources(psnip-builtin INTERFACE "${psnip_SOURCE_DIR}/builtin/builtin.h")
  target_include_directories(psnip-builtin SYSTEM INTERFACE "${psnip_SOURCE_DIR}")
endif ()

FetchContent_Declare(tiny-aes
	GIT_REPOSITORY https://github.com/kokke/tiny-AES-c
  PATCH_COMMAND git apply "${${PROJECT_NAME}_SOURCE_DIR}/a.patch" || git apply "${${PROJECT_NAME}_SOURCE_DIR}/a.patch" -R --check && echo already applied
)
FetchContent_MakeAvailable(tiny-aes)

CPMGetPackage(explicit-memset)

####################
# Section: Targets #
####################

add_executable(fortuna-rng main.c)
target_link_libraries(fortuna-rng PRIVATE monocypher fortuna-aes-accumulator psnip-check internal-hexdump)

add_library(fortuna-accumulator accumulator.c)
target_link_libraries(fortuna-accumulator
	PUBLIC psnip-clock fortuna-pool
	PRIVATE psnip-check explicit-memset::explicit-memset
)
target_include_directories(fortuna-accumulator PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_library(fortuna-aes-generator aes-generator.c)
target_link_libraries(fortuna-aes-generator
	PRIVATE shad256 tiny-aes psnip-check explicit-memset::explicit-memset
	PUBLIC u128
)
target_include_directories(fortuna-aes-generator PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
add_subdirectory(shad256)

add_library(fortuna-pool pool.c)
target_link_libraries(fortuna-pool PUBLIC shad256 u128)
target_include_directories(fortuna-pool PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_library(u128 u128.c)
target_sources(u128 PRIVATE u128.c u128.h)
target_link_libraries(u128 PUBLIC psnip-endian psnip-builtin)

add_library(fortuna-aes-accumulator aes-accumulator.c)
target_link_libraries(fortuna-aes-accumulator PUBLIC fortuna-aes-generator fortuna-accumulator)
target_include_directories(fortuna-aes-accumulator PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_library(internal-hexdump hexdump.c)
target_include_directories(internal-hexdump PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(chacha20)

####################
# Section: Testing #
####################

if (${PROJECT_NAME}_BUILD_TESTING)
  add_subdirectory(tests)
endif ()

