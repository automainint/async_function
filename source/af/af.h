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

enum af_request {
  af_request_resume,
  af_request_join,
  af_request_resume_and_join
};

typedef struct {
  int _;
} af_void;

typedef void (*af_state_machine)(void *self_void_, int request_);

typedef struct {
  void *state;
  void (*resume)(void *state, af_state_machine state_machine_,
                 void *coro_state_);
  void (*join)(void *state, void *coro_state_);
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

#define AF_DECL(name_) \
  void name_##_coro_(void *self_void_, int request_)

#define CORO_IMPL(name_)                                   \
  AF_DECL(name_) {                                         \
    struct name_##_coro_state_ *self =                     \
        (struct name_##_coro_state_ *) self_void_;         \
    if (self->_status == af_status_suspended) {            \
      if (self->_context.resume != NULL &&                 \
          request_ == af_request_resume)                   \
        self->_context.resume(self->_context.state,        \
                              self->_state_machine, self); \
      else if (self->_context.join != NULL &&              \
               request_ == af_request_join)                \
        self->_context.join(self->_context.state, self);   \
      else if (request_ == af_request_join ||              \
               request_ == af_request_resume_and_join) {   \
        self->_status = af_status_ready;                   \
        self->_state_machine(self, af_request_join);       \
      }                                                    \
      return;                                              \
    }                                                      \
    if (self->_status != af_status_ready ||                \
        request_ != af_request_join)                       \
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

#define AF_AWAIT(promise_)                                     \
  {                                                            \
    case AF_LINE():                                            \
      if ((promise_)._status != af_status_finished) {          \
        self->_status = af_status_waiting;                     \
        self->_index  = AF_LINE();                             \
        (promise_)._state_machine(&(promise_),                 \
                                  af_request_resume_and_join); \
      }                                                        \
      if ((promise_)._status != af_status_finished) {          \
        self->_status = af_status_suspended;                   \
        return;                                                \
      } else                                                   \
        self->_status = af_status_running;                     \
  }

#define AF_YIELD_AWAIT(promise_)                               \
  {                                                            \
    case AF_LINE():                                            \
      if ((promise_)._status != af_status_finished) {          \
        self->_status = af_status_suspended;                   \
        self->_index  = AF_LINE();                             \
        (promise_)._state_machine(&(promise_),                 \
                                  af_request_resume_and_join); \
        self->return_value = (promise_).return_value;          \
        return;                                                \
      }                                                        \
  }

#define AF_TYPE(coro_) struct coro_##_coro_state_

#define AF_INITIAL(coro_)                      \
  ._status = af_status_suspended, ._index = 0, \
  ._state_machine = coro_##_coro_,             \
  ._context       = { .state = NULL, .resume = NULL, .join = NULL }

#define AF_CREATE(promise_, coro_, ...) \
  AF_TYPE(coro_)                        \
  promise_ = { AF_INITIAL(coro_), __VA_ARGS__ }

#define AF_INIT(promise_, coro_, ...)        \
  {                                          \
    AF_CREATE(_af_temp, coro_, __VA_ARGS__); \
    (promise_) = _af_temp;                   \
  }

#define AF_RESUME(promise_) \
  (promise_)._state_machine(&(promise_), af_request_resume)

#define AF_RESUME_N(promises_, size_)                       \
  for (int af_index_ = 0; af_index_ < (size_); af_index_++) \
  AF_RESUME((promises_)[af_index_])

#define AF_JOIN(promise_)                                   \
  ((promise_)._state_machine(&(promise_), af_request_join), \
   (promise_).return_value)

#define AF_JOIN_N(promises_, size_)                         \
  for (int af_index_ = 0; af_index_ < (size_); af_index_++) \
  AF_JOIN((promises_)[af_index_])

#define AF_RESUME_AND_JOIN(promise_)                      \
  ((promise_)._state_machine(&(promise_),                 \
                             af_request_resume_and_join), \
   (promise_).return_value)

#define AF_RESUME_AND_JOIN_N(promises_, size_) \
  AF_RESUME_N((promises_), (size_));           \
  AF_JOIN_N((promises_), (size_))

#define AF_RESUME_ALL(promises_) \
  AF_RESUME_N((promises_), sizeof(promises_) / sizeof((promises_)[0]))

#define AF_JOIN_ALL(promises_) \
  AF_JOIN_N((promises_), sizeof(promises_) / sizeof((promises_)[0]))

#define AF_RESUME_AND_JOIN_ALL(promises_) \
  AF_RESUME_AND_JOIN_N((promises_),       \
                       sizeof(promises_) / sizeof((promises_)[0]))

#define AF_FINISHED(promise_) \
  ((promise_)._status == af_status_finished)

#define AF_FINISHED_N(return_, promises_, size_)              \
  {                                                           \
    (return_) = true;                                         \
    for (int af_index_ = 0; af_index_ < (size_); af_index_++) \
      if (!AF_FINISHED((promises_)[af_index_])) {             \
        (return_) = false;                                    \
        break;                                                \
      }                                                       \
  }

#define AF_FINISHED_ALL(return_, promises_, size_) \
  AF_FINISHED_N((return_), (promises_),            \
                sizeof(promises_) / sizeof((promises_)[0]))

#define AF_AWAIT_N(promises_, size_)                 \
  {                                                  \
    case AF_LINE():                                  \
      self->_status = af_status_waiting;             \
      self->_index  = AF_LINE();                     \
      AF_RESUME_AND_JOIN_N((promises_), (size_));    \
      bool af_done_;                                 \
      AF_FINISHED_N(af_done_, (promises_), (size_)); \
      if (!af_done_) {                               \
        self->_status = af_status_suspended;         \
        return;                                      \
      } else                                         \
        self->_status = af_status_running;           \
  }

#define AF_AWAIT_ALL(promises_) \
  AF_AWAIT_N((promises_), sizeof(promises_) / sizeof((promises_)[0]))

#ifdef __cplusplus
}
#endif

#endif
