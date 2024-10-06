add_library(shad256 shad256.c)
target_link_libraries(shad256
	PUBLIC amosnier-sha-2
)
target_include_directories(shad256 PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

