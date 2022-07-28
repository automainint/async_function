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

TEST("coroutine create and destroy") {
  AF_CREATE(promise, test_foo);
}

TEST("coroutine resume and get returned value") {
  AF_CREATE(promise, test_foo);
  REQUIRE(AF_RESUME(promise) == 42);
}

TEST("coroutine suspend") {
  AF_CREATE(promise, test_bar);
  REQUIRE(AF_RESUME(promise) == 0);
  REQUIRE(AF_RESUME(promise) == 42);
}

TEST("coroutine generator") {
  AF_CREATE(promise, test_gen, .min = 10, .max = 15);
  for (int i = 0; i <= 5; i++) REQUIRE(AF_RESUME(promise) == 10 + i);
}

TEST("coroutine status finished") {
  AF_CREATE(promise, test_bar);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine task") {
  AF_CREATE(promise, test_task);
  AF_RESUME(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine nested task") {
  AF_CREATE(promise, test_nest_task);
  AF_RESUME(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(AF_FINISHED(promise));
}

TEST("coroutine nested generator") {
  AF_CREATE(promise, test_nest_generator);
  REQUIRE(AF_RESUME(promise) == 1);
  REQUIRE(AF_RESUME(promise) == 2);
  REQUIRE(AF_RESUME(promise) == 3);
  REQUIRE(!AF_FINISHED(promise));
  AF_RESUME(promise);
  REQUIRE(AF_FINISHED(promise));
}
