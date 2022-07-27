cmake_minimum_required(VERSION 3.16)
set(NAME af-integration-fetch-content)
project(${NAME} C)

include(FetchContent)
FetchContent_Declare(
  af
  GIT_REPOSITORY https://github.com/automainint/async_function.git
  GIT_TAG dev)
set(AF_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(af)

add_executable(${NAME})
target_sources(${NAME} PRIVATE main.c)
target_link_libraries(${NAME} PRIVATE af::af)

enable_testing()
add_test(NAME ${NAME}-run COMMAND ${NAME})
set_tests_properties(${NAME}-run PROPERTIES TIMEOUT "15")
