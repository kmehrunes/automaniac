cmake_minimum_required(VERSION 3.5)

project(automaniac)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14 -Wl,-rpath=${PROJECT_SOURCE_DIR}/boost_bin -L${PROJECT_SOURCE_DIR}/boost_bin -lboost_system -lboost_filesystem -lpthread")

set(source_dir "${PROJECT_SOURCE_DIR}/src/")
set(include_dir "${PROJECT_SOURCE_DIR}/includes/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB source_files "${source_dir}/*.cpp")

add_executable(automaniac ${source_files})

target_include_directories(automaniac PUBLIC ${include_dir})

## Add 'catch-test' target to run tests using CATCH2
add_custom_target(catch-test COMMAND bash test)
