cmake_minimum_required(VERSION 3.5)

# Set the project name
project(CSP)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Support for test names
if(POLICY CMP0110)
  cmake_policy(SET CMP0110 NEW)
endif()

# Add the executable
add_executable(solve-queens src/solve-queens.c)
target_link_libraries(solve-queens PRIVATE csp)

# Add the library
add_library(csp SHARED
  src/csp.c
  src/csp.inc
  src/forward-checking.c
  src/heuristics.c
)
target_include_directories(csp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link the library to the executable
target_link_libraries(solve-queens PRIVATE csp)

# Find test files
file(GLOB FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/test-*.c")

enable_testing()

find_program(VALGRIND valgrind)

# Add tests
foreach(FILENAME ${FILES})
  get_filename_component(SRC_NAME ${FILENAME} NAME)
  get_filename_component(TEST ${FILENAME} NAME_WE)
  add_executable(${TEST} ${FILENAME})
  target_link_libraries(${TEST} PRIVATE csp)
  add_test(NAME ${TEST} COMMAND $<TARGET_FILE:${TEST}>)
  if(VALGRIND)
    add_test(
      NAME "${TEST}[valgrind]"
      COMMAND ${VALGRIND} --leak-check=full --quiet --error-exitcode=1 $<TARGET_FILE:${TEST}>
    )
  endif()
endforeach()

# Add CPack
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "CSP")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)
