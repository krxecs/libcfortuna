add_library(fortuna-chacha20-generator chacha20-generator.c)
target_link_libraries(fortuna-chacha20-generator
	PRIVATE shad256 psnip-check explicit-memset::explicit-memset monocypher
)
target_include_directories(fortuna-chacha20-generator PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_library(fortuna-chacha20-accumulator chacha20-accumulator.c)
target_link_libraries(fortuna-chacha20-accumulator
  PUBLIC fortuna-accumulator fortuna-chacha20-generator
)
target_include_directories(fortuna-chacha20-accumulator PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_executable(main1 main1.c)
target_link_libraries(main1 fortuna-chacha20-generator fortuna-chacha20-accumulator)
