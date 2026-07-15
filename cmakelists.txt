cmake_minimum_required(VERSION 3.10)
project(ArrowCSS VERSION 0.1.0 LANGUAGES C)

add_subdirectory(external/ArenaAllocator)

add_library(ArrowCSS STATIC 
src/CSSParser.c
    )

target_sources(ArrowCSS PRIVATE
    src/Lexer.c
    src/CSSParser.c
    src/Generator.c
)

target_include_directories(ArrowCSS PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ArrowCSS PUBLIC MemoryArena)

# -----------------------------
# Testing setup
# -----------------------------
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    enable_testing()

    file(GLOB TEST_SOURCES "test/test_*.c")

    foreach(TEST_FILE ${TEST_SOURCES})

        # Get test name from file name (without extension)
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE} test/testing.c)

        # Link the ArrowCSS library to the test executable
        target_link_libraries(${TEST_NAME} PRIVATE ArrowCSS)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    endforeach()

endif()