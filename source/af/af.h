#ifndef AF_AF_H
#define AF_AF_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum af_status {
  af_status_suspended,
  af_status_ready,
  af_status_running,
  af_status_waiting,
  af_status_finished
};

typedef struct {
  int _;
} af_void;

typedef void (*af_state_machine)(void *self_void_);

typedef struct {
  void *state;
  void (*resume)(void *state, af_state_machine state_machine_,
                 void *self_);
} af_execution_context;

#ifndef AF_DISABLE_SELF_SHORTCUT
#  define af self->
#endif

#define AF_STATE(ret_type_, name_, ...)  \
  struct name_##_coro_state_ {           \
    int                  _status;        \
    int                  _index;         \
    af_state_machine     _state_machine; \
    af_execution_context _context;       \
    ret_type_            return_value;   \
    __VA_ARGS__                          \
  }

#define AF_DECL(name_) void name_##_coro_(void *self_void_)

#define CORO_IMPL(name_)                                   \
  AF_DECL(name_) {                                         \
    struct name_##_coro_state_ *self =                     \
        (struct name_##_coro_state_ *) self_void_;         \
    if (self->_status == af_status_suspended) {            \
      if (self->_context.resume != NULL)                   \
        self->_context.resume(self->_context.state,        \
                              self->_state_machine, self); \
      else {                                               \
        self->_status = af_status_ready;                   \
        self->_state_machine(self);                        \
      }                                                    \
      return;                                              \
    }                                                      \
    if (self->_status != af_status_ready)                  \
      return;                                              \
    self->_status = af_status_running;                     \
    switch (self->_index) {                                \
      case 0:;

#define AF_LINE() __LINE__

#define CORO_END                      \
  }                                   \
  self->_status = af_status_finished; \
  }

#define CORO_DECL(ret_type_, name_, ...)   \
  AF_STATE(ret_type_, name_, __VA_ARGS__); \
  AF_DECL(name_)

#define CORO(ret_type_, name_, ...)        \
  AF_STATE(ret_type_, name_, __VA_ARGS__); \
  CORO_IMPL(name_)

#define CORO_DECL_VOID(name_, ...) \
  CORO_DECL(af_void, name_, __VA_ARGS__)

#define CORO_VOID(name_, ...) CORO(af_void, name_, __VA_ARGS__)

#define AF_YIELD(...)                         \
  {                                           \
    self->_status      = af_status_suspended; \
    self->_index       = AF_LINE();           \
    self->return_value = __VA_ARGS__;         \
    return;                                   \
    case AF_LINE():;                          \
  }

#define AF_YIELD_VOID                    \
  {                                      \
    self->_status = af_status_suspended; \
    self->_index  = AF_LINE();           \
    return;                              \
    case AF_LINE():;                     \
  }

#define AF_RETURN(...)                       \
  {                                          \
    self->_status      = af_status_finished; \
    self->_index       = AF_LINE();          \
    self->return_value = __VA_ARGS__;        \
    return;                                  \
  }

#define AF_RETURN_VOID                  \
  {                                     \
    self->_status = af_status_finished; \
    self->_index  = AF_LINE();          \
    return;                             \
  }

#define AF_AWAIT(promise_)                            \
  {                                                   \
    case AF_LINE():                                   \
      if ((promise_)._status != af_status_finished) { \
        self->_status = af_status_waiting;            \
        self->_index  = AF_LINE();                    \
        (promise_)._state_machine(&(promise_));       \
      }                                               \
      if ((promise_)._status != af_status_finished) { \
        self->_status = af_status_suspended;          \
        return;                                       \
      } else                                          \
        self->_status = af_status_running;            \
  }

#define AF_YIELD_AWAIT(promise_)                      \
  {                                                   \
    case AF_LINE():                                   \
      if ((promise_)._status != af_status_finished) { \
        self->_status = af_status_suspended;          \
        self->_index  = AF_LINE();                    \
        (promise_)._state_machine(&(promise_));       \
        self->return_value = (promise_).return_value; \
        return;                                       \
      }                                               \
  }

#define AF_TYPE(coro_) struct coro_##_coro_state_

#define AF_INITIAL(coro_)                      \
  ._status = af_status_suspended, ._index = 0, \
  ._state_machine = coro_##_coro_,             \
  ._context       = { .state = NULL, .resume = NULL }

#define AF_CREATE(promise_, coro_, ...) \
  AF_TYPE(coro_)                        \
  promise_ = { AF_INITIAL(coro_), __VA_ARGS__ }

#define AF_INIT(promise_, coro_, ...)        \
  {                                          \
    AF_CREATE(_af_temp, coro_, __VA_ARGS__); \
    (promise_) = _af_temp;                   \
  }

#define AF_RESUME(promise_) \
  ((promise_)._state_machine(&(promise_)), (promise_).return_value)

#define AF_FINISHED(promise_) \
  ((promise_)._status == af_status_finished)

#ifdef __cplusplus
}
#endif

#endif
