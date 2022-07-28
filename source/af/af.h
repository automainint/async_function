#ifndef AF_AF_H
#define AF_AF_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum af_status {
  af_status_suspended = 0,
  af_status_running,
  af_status_finished
};

#define af self->

#define AF_STATE(ret_type_, name_, ...)                             \
  struct name_##_coro_state_ {                                      \
    int       _status;                                              \
    int       _index;                                               \
    ret_type_ return_value;                                         \
    ret_type_ (*_state_machine)(struct name_##_coro_state_ * self); \
    __VA_OPT__(__VA_ARGS__;)                                        \
  }

#define AF_DECL(ret_type_, name_) \
  ret_type_ name_##_coro_(struct name_##_coro_state_ *self)

#define AF_IMPL(ret_type_, name_)             \
  AF_DECL(ret_type_, name_) {                 \
    if (self->_status != af_status_suspended) \
      return self->return_value;              \
    self->_status = af_status_running;        \
    switch (self->_index) {                   \
      case 0:;

#define AF_LINE() __LINE__

#define CORO_END                      \
  }                                   \
  self->_status = af_status_finished; \
  self->_index  = AF_LINE();          \
  return self->return_value;          \
  }

#define CORO(ret_type_, name_, ...)        \
  AF_STATE(ret_type_, name_, __VA_ARGS__); \
  AF_IMPL(ret_type_, name_)

#define AF_YIELD(...)                             \
  {                                               \
    self->_status = af_status_suspended;          \
    self->_index  = AF_LINE();                    \
    __VA_OPT__(self->return_value = __VA_ARGS__); \
    return self->return_value;                    \
    case AF_LINE():;                              \
  }

#define AF_RETURN(...)                            \
  {                                               \
    self->_status = af_status_finished;           \
    self->_index  = AF_LINE();                    \
    __VA_OPT__(self->return_value = __VA_ARGS__); \
    return self->return_value;                    \
  }

#define AF_TYPE(coro_) struct coro_##_coro_state_

#define AF_INIT(promise_, coro_)                   \
  (promise_)._status        = af_status_suspended; \
  (promise_)._index         = 0;                   \
  (promise_)._state_machine = coro_##_coro_

#define AF_CREATE(promise_, coro_, ...)                  \
  AF_TYPE(coro_) promise_ __VA_OPT__(= { __VA_ARGS__ }); \
  AF_INIT(promise_, coro_)

#define AF_DESTROY(promise_)

#define AF_RESUME(promise_) promise_._state_machine(&(promise_))

#define AF_FINISHED(promise_) \
  ((promise_)._status == af_status_finished)

#ifdef __cplusplus
}
#endif

#endif
