cmake_minimum_required(VERSION 3.14)
project(dsl_codegen_tests LANGUAGES CXX)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test: XmlParser
add_executable(TestXmlParser TestXmlParser.cpp)
target_link_libraries(TestXmlParser PRIVATE dsl_codegen_lib gtest gtest_main)
add_test(NAME TestXmlParser COMMAND TestXmlParser)

# Test: BlockGraph
add_executable(TestBlockGraph TestBlockGraph.cpp)
target_link_libraries(TestBlockGraph PRIVATE dsl_codegen_lib gtest gtest_main)
add_test(NAME TestBlockGraph COMMAND TestBlockGraph)

# Test: Scheduler
add_executable(TestScheduler TestScheduler.cpp)
target_link_libraries(TestScheduler PRIVATE dsl_codegen_lib gtest gtest_main)
add_test(NAME TestScheduler COMMAND TestScheduler)

# Test: CodeGenerator
add_executable(TestCodeGenerator TestCodeGenerator.cpp)
target_link_libraries(TestCodeGenerator PRIVATE dsl_codegen_lib gtest gtest_main)
add_test(NAME TestCodeGenerator COMMAND TestCodeGenerator)

# Integration test (C++ based, cross-platform)
add_executable(TestIntegration TestIntegration.cpp)
target_link_libraries(TestIntegration PRIVATE dsl_codegen_lib gtest gtest_main)
target_compile_definitions(TestIntegration PRIVATE
    FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
add_test(NAME TestIntegration COMMAND TestIntegration)
