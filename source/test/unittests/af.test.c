#include "../../af/af.h"

#define AF_TEST_FILE af_test
#include "test.h"

CORO(int, test_foo) {
  AF_RETURN(42);
}
CORO_END

CORO(int, test_bar) {
  AF_YIELD_VOID;
  AF_RETURN(42);
}
CORO_END

CORO(int, test_gen, int i; int min; int max;) {
  for (af i = af min; af i < af max; af i++) AF_YIELD(af i);
  AF_RETURN(af max);
}
CORO_END

CORO_VOID(test_task) {
  AF_YIELD_VOID;
  AF_YIELD_VOID;
  AF_RETURN_VOID;
}
CORO_END

CORO_VOID(test_nest_task, AF_TYPE(test_task) promise;) {
  AF_INIT(af promise, test_task);
  AF_AWAIT(af promise);
  AF_AWAIT(af promise);
  AF_AWAIT(af promise);
}
CORO_END

CORO(int, test_nest_generator, AF_TYPE(test_gen) promise;) {
  AF_INIT(af promise, test_gen, .min = 1, .max = 3);
  AF_YIELD_AWAIT(af promise);
}
CORO_END

CORO(int, test_join_multiple, AF_TYPE(test_bar) promises[3];) {
  for (int i = 0; i < 3; i++)
    AF_INIT(af promises[i], test_bar, .return_value = 0);
  AF_RESUME_AND_JOIN_ALL(af promises);
  AF_RETURN(af promises[0].return_value +
            af promises[1].return_value +
            af promises[2].return_value);
}
CORO_END

CORO(int, test_await_multiple, AF_TYPE(test_bar) promises[3];) {
  for (int i = 0; i < 3; i++)
    AF_INIT(af promises[i], test_bar, .return_value = 0);
  AF_AWAIT_ALL(af promises);
  AF_RETURN(af promises[0].return_value +
            af promises[1].return_value +
            af promises[2].return_value);
}
CORO_END

void test_execute_lazy(void *_, void *coro, int request) {
  if (request == af_request_resume)
    return;
  AF_EXECUTE(coro);
}

void test_execute_immediate(void *_, void *coro, int request) {
  if (request == af_request_join)
    return;
  AF_EXECUTE(coro);
}

TEST("coroutine create") {
  AF_CREATE(promise, test_foo);
  REQUIRE(!AF_FINISHED(promise));
}

TEST("coroutine init") {
  AF_TYPE(test_foo) promise;
  AF_INIT(promise, test_foo);
  REQUIRE(!AF_FINISHED(promise));
}

TEST("coroutine init with value") {
  AF_TYPE(test_foo) promise;
  AF_INIT(promise, test_foo, .return_value = 42);
  REQUIRE(promise.return_value == 42);
  REQUIRE(!AF_FINISHED(promise));
}

TEST("coroutine resume") {
  AF_CREATE(promise, test_foo, .return_value = -1);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == -1);
  REQUIRE(!AF_FINISHED(promise));
}

TEST("coroutine resume and join") {
  AF_CREATE(promise, test_foo);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 42);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine resume and join manually") {
  AF_CREATE(promise, test_foo);
  AF_RESUME(promise);
  REQUIRE(AF_JOIN(promise) == 42);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine suspend") {
  AF_CREATE(promise, test_bar, .return_value = 0);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 0);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 42);
}

TEST("coroutine generator") {
  AF_CREATE(promise, test_gen, .min = 10, .max = 15);
  for (int i = 0; i <= 5; i++)
    REQUIRE(AF_RESUME_AND_JOIN(promise) == 10 + i);
}

TEST("coroutine status finished") {
  AF_CREATE(promise, test_bar);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine task") {
  AF_CREATE(promise, test_task);
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine nested task") {
  AF_CREATE(promise, test_nest_task);
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine nested generator") {
  AF_CREATE(promise, test_nest_generator);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 1);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 2);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 3);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME_AND_JOIN(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine join multiple") {
  AF_CREATE(promise, test_join_multiple);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 0);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine await multiple") {
  AF_CREATE(promise, test_await_multiple);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 0);
  REQUIRE(AF_RESUME_AND_JOIN(promise) == 42 * 3);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine custom execution context lazy") {
  AF_CREATE(promise, test_foo, .return_value = 0);
  AF_EXECUTION_CONTEXT(promise, .state = NULL,
                       .execute = test_execute_lazy);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == 0);
  REQUIRE(!AF_FINISHED(promise));
  AF_JOIN(promise);
  REQUIRE(promise.return_value == 42);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine custom execution context immediate") {
  AF_CREATE(promise, test_foo, .return_value = 0);
  AF_EXECUTION_CONTEXT(promise, .state = NULL,
                       .execute = test_execute_immediate);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == 42);
  REQUIRE(AF_FINISHED(promise));
  AF_JOIN(promise);
  REQUIRE(promise.return_value == 42);
  REQUIRE(AF_FINISHED(promise));
}
