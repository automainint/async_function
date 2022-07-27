cmake_minimum_required(VERSION 3.16)

option(AF_ENABLE_LIBRARY "Enable library" ON)
option(AF_ENABLE_TESTING "Enable testing" ON)

set(AF_PROJECT af)
set(AF_LIBRARY af)
set(AF_TEST af_test)
set(AF_TARGETS af-targets)
set(AF_UNITTESTS af-unittests)

if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
  set(CMAKE_BUILD_PARALLEL_LEVEL 4)
endif()

project(
  ${AF_PROJECT}
  VERSION 0.1.1
  DESCRIPTION "Coroutine library for C"
  LANGUAGES C)

if(AF_ENABLE_LIBRARY OR AF_ENABLE_TESTING)
  add_library(${AF_LIBRARY} STATIC)
  set_target_properties(${AF_LIBRARY} PROPERTIES PREFIX "")
  add_library(${AF_PROJECT}::${AF_LIBRARY} ALIAS ${AF_LIBRARY})
  target_include_directories(
    ${AF_LIBRARY} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
  target_compile_features(${AF_LIBRARY} PUBLIC c_std_99)
endif()

enable_testing()

if(AF_ENABLE_TESTING)
  add_executable(${AF_TEST})
  add_executable(${AF_PROJECT}::${AF_TEST} ALIAS ${AF_TEST})
  target_compile_features(${AF_TEST} PRIVATE c_std_99)
  target_link_libraries(${AF_TEST} PRIVATE ${AF_LIBRARY})

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
      ${AF_LIBRARY} PUBLIC
      -fsanitize=undefined,address
      --coverage -O0 -g)
    target_link_options(
      ${AF_LIBRARY} PUBLIC
      -fsanitize=undefined,address
      --coverage)
  endif()

  add_test(
    NAME ${AF_UNITTESTS}
    COMMAND ${AF_TEST} --test)

  set_tests_properties(
    ${AF_UNITTESTS}
    PROPERTIES
    TIMEOUT "30")
endif()

add_subdirectory(source)

if(AF_ENABLE_LIBRARY)
  include(GNUInstallDirs)

  file(
    GLOB_RECURSE headers_
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/af/*.h)

  foreach(path_ ${headers_})
    install(
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/${path_}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/af)
  endforeach()

  unset(headers_)
  unset(path_)

  install(
    TARGETS ${AF_LIBRARY}
    EXPORT ${AF_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(
    EXPORT ${AF_TARGETS}
    FILE ${AF_TARGETS}.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

  export(
    EXPORT ${AF_TARGETS}
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${AF_TARGETS}.cmake"
    NAMESPACE ${PROJECT_NAME}::)

  include(CMakePackageConfigHelpers)

  string(TOLOWER ${PROJECT_NAME} project_lower_)

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${project_lower_}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${project_lower_}-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion)

  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${project_lower_}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${project_lower_}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  unset(project_lower_)
endif()
