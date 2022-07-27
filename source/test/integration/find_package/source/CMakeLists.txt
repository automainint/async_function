cmake_minimum_required(VERSION 3.16)
set(NAME af-integration-find-package)
project(${NAME} C)

find_package(af REQUIRED)

add_executable(${NAME})
target_sources(${NAME} PRIVATE main.c)
target_link_libraries(${NAME} PRIVATE af::af)
install(TARGETS ${NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
