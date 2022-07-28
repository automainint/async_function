#include "../../af/af.h"

#define AF_TEST_FILE af_test
#include "test.h"

CORO(int, foo) {
  AF_RETURN(42);
}
CORO_END

CORO(int, bar) {
  AF_YIELD();
  AF_RETURN(42);
}
CORO_END

CORO(int, gen, int i; int min; int max) {
  for (af i = af min; af i < af max; af i++) AF_YIELD(af i);
}
CORO_END

TEST("coroutine create and destroy") {
  AF_CREATE(promise, foo);
  AF_DESTROY(promise);
}

TEST("coroutine resume and get returned value") {
  AF_CREATE(promise, foo);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == 42);
  AF_DESTROY(promise);
}

TEST("coroutine suspend") {
  AF_CREATE(promise, bar);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == 0);
  AF_RESUME(promise);
  REQUIRE(promise.return_value == 42);
  AF_DESTROY(promise);
}

TEST("coroutine generator") {
  AF_CREATE(promise, gen);
  promise.min = 10;
  promise.max = 15;
  for (int i = 0; i < 5; i++) REQUIRE(AF_RESUME(promise) == 10 + i);
  AF_DESTROY(promise);
}
