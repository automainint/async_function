target_sources(
  ${AF_TEST}
    PRIVATE
      main.test.c af.test.c
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/test.h>)
