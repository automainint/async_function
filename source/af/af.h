#ifndef AF_AF_H
#define AF_AF_H

#ifdef __cplusplus
extern "C" {
#endif

enum af_request {
  af_request_resume,
  af_request_join,
  af_request_resume_and_join,
  af_request_execute
};

typedef struct {
  int _;
} af_void;

typedef void (*af_state_machine)(void *self_void_, int request_);
typedef void (*af_execute)(void *state, void *coro_state,
                           int request);

typedef struct {
  long long  _internal;
  void      *state;
  af_execute execute;
} af_execution_context;

typedef struct {
  int                  _index;
  af_state_machine     _state_machine;
  af_execution_context _context;
} af_type_void;

#ifndef AF_DISABLE_SELF_SHORTCUT
#  define af self->
#endif

#define AF_INTERNAL(coro_) (*((af_type_void *) (coro_)))

#define AF_STATE(ret_type_, name_, ...)  \
  struct name_##_coro_state_ {           \
    int                  _index;         \
    af_state_machine     _state_machine; \
    af_execution_context _context;       \
    ret_type_            return_value;   \
    __VA_ARGS__                          \
  }

#define AF_DECL(name_) \
  void name_##_coro_(void *self_void_, int request_)

#define CORO_IMPL(name_)                                         \
  AF_DECL(name_) {                                               \
    struct name_##_coro_state_ *self =                           \
        (struct name_##_coro_state_ *) self_void_;               \
    if (request_ != af_request_execute) {                        \
      if (self->_context.execute != NULL)                        \
        self->_context.execute(self->_context.state, self_void_, \
                               request_);                        \
      else if (request_ == af_request_join ||                    \
               request_ == af_request_resume_and_join)           \
        self->_state_machine(self_void_, af_request_execute);    \
      return;                                                    \
    }                                                            \
    switch (self->_index) {                                      \
      case 0:;

#define AF_LINE() __LINE__

#define CORO_END     \
  }                  \
  self->_index = -1; \
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

#define AF_YIELD(...)                 \
  {                                   \
    self->_index       = AF_LINE();   \
    self->return_value = __VA_ARGS__; \
    return;                           \
    case AF_LINE():;                  \
  }

#define AF_YIELD_VOID         \
  {                           \
    self->_index = AF_LINE(); \
    return;                   \
    case AF_LINE():;          \
  }

#define AF_RETURN(...)                \
  {                                   \
    self->_index       = -1;          \
    self->return_value = __VA_ARGS__; \
    return;                           \
  }

#define AF_RETURN_VOID \
  {                    \
    self->_index = -1; \
    return;            \
  }

#define AF_AWAIT(promise_)                                     \
  {                                                            \
    case AF_LINE():                                            \
      if ((promise_)._index != -1) {                           \
        self->_index = AF_LINE();                              \
        (promise_)._state_machine(&(promise_),                 \
                                  af_request_resume_and_join); \
      }                                                        \
      if ((promise_)._index != -1)                             \
        return;                                                \
  }

#define AF_YIELD_AWAIT(promise_)                               \
  {                                                            \
    case AF_LINE():                                            \
      if ((promise_)._index != -1) {                           \
        self->_index = AF_LINE();                              \
        (promise_)._state_machine(&(promise_),                 \
                                  af_request_resume_and_join); \
        self->return_value = (promise_).return_value;          \
        return;                                                \
      }                                                        \
  }

#define AF_TYPE(coro_) struct coro_##_coro_state_

#define AF_INITIAL(coro_)                       \
  ._index = 0, ._state_machine = coro_##_coro_, \
  ._context = { .state = NULL, .execute = NULL }

#define AF_CREATE(promise_, coro_, ...) \
  AF_TYPE(coro_)                        \
  promise_ = { AF_INITIAL(coro_), __VA_ARGS__ }

#define AF_INIT(promise_, coro_, ...)        \
  {                                          \
    AF_CREATE(af_temp_, coro_, __VA_ARGS__); \
    (promise_) = af_temp_;                   \
  }

#define AF_EXECUTION_CONTEXT(promise_, ...)                          \
  {                                                                  \
    af_execution_context af_temp_ = { ._internal = 0, __VA_ARGS__ }; \
    (promise_)._context           = af_temp_;                        \
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

#define AF_FINISHED(promise_) ((promise_)._index == -1)

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
      self->_index = AF_LINE();                      \
      AF_RESUME_AND_JOIN_N((promises_), (size_));    \
      bool af_done_;                                 \
      AF_FINISHED_N(af_done_, (promises_), (size_)); \
      if (!af_done_)                                 \
        return;                                      \
  }

#define AF_AWAIT_ALL(promises_) \
  AF_AWAIT_N((promises_), sizeof(promises_) / sizeof((promises_)[0]))

#define AF_EXECUTE(coro_state_) \
  AF_INTERNAL(coro_state_)._state_machine(coro, af_request_execute)

#ifdef __cplusplus
}
#endif

#endif
