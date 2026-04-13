cmake_minimum_required(VERSION 3.14)
project(dsl-codegen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Fetch pugixml
include(FetchContent)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.14
)
FetchContent_MakeAvailable(pugixml)

# Main library
add_library(dsl_codegen_lib
    src/parser/XmlParser.cpp
    src/graph/BlockGraph.cpp
    src/scheduler/Scheduler.cpp
    src/codegen/CodeGenerator.cpp
)

target_include_directories(dsl_codegen_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(dsl_codegen_lib PUBLIC pugixml)

# CLI executable
add_executable(dsl-codegen src/cli/main.cpp)
target_link_libraries(dsl-codegen PRIVATE dsl_codegen_lib)

# Install target: copy only the executable + static header to bin/
install(TARGETS dsl-codegen RUNTIME DESTINATION bin)

# Enable testing
enable_testing()

# Tests (optional subdirectory)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
