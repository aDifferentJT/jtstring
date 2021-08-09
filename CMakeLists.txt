
cmake_minimum_required(VERSION 3.10)

project(jtstring)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(compare_to_std test/compare_to_std.cpp)

add_subdirectory("rapidcheck")
target_link_libraries(compare_to_std rapidcheck)

target_compile_options(compare_to_std PUBLIC -fsanitize=address -fprofile-instr-generate -fcoverage-mapping)
target_link_options(compare_to_std PUBLIC -fsanitize=address -fprofile-instr-generate -fcoverage-mapping)

add_custom_target(check ALL compare_to_std)

add_custom_target(coverage llvm-profdata-12 merge -sparse default.profraw -o compare_to_std.profdata
  COMMAND llvm-cov-12 show --ignore-filename-regex="rapidcheck/*|test/*" ./compare_to_std -instr-profile=compare_to_std.profdata
  COMMAND llvm-cov-12 report --ignore-filename-regex="rapidcheck/*|test/*" ./compare_to_std -instr-profile=compare_to_std.profdata
  DEPENDS check
  )


