target_sources(
  ${AF_LIBRARY}
    PRIVATE
      af.c
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/af.h>)
