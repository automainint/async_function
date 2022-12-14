#ifndef AF_TEST_H
#define AF_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifndef AF_TEST_FILE
#  define af_test
#endif

#ifndef AF_TESTS_SIZE_LIMIT
#  define AF_TESTS_SIZE_LIMIT 0x1000
#endif

#ifndef AF_TEST_ASSERTIONS_LIMIT
#  define AF_TEST_ASSERTIONS_LIMIT 0x50
#endif

#ifndef AF_TEST_STRING_SIZE
#  define AF_TEST_STRING_SIZE 0x100
#endif

typedef void (*af_test_report)(int, char const *file, int line, bool);
typedef void (*af_test_function)(int, af_test_report);

struct af_test_case {
  char             test_name[AF_TEST_STRING_SIZE];
  af_test_function test_fn;
  int              assertions;
  char const      *file[AF_TEST_ASSERTIONS_LIMIT];
  int              line[AF_TEST_ASSERTIONS_LIMIT];
  bool             status[AF_TEST_ASSERTIONS_LIMIT];
};

struct af_tests_list {
  int                 size;
  struct af_test_case tests[AF_TESTS_SIZE_LIMIT];
};

extern struct af_tests_list af_tests_list;

#ifdef _MSC_VER
#  pragma section(".CRT$XCU", read)
#  define AF_TEST_ON_START_2(f, p)                                 \
    static void f(void);                                           \
    __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f;       \
    __pragma(comment(linker, "/include:" p #f "_")) static void f( \
        void)
#  ifdef _WIN64
#    define AF_TEST_ON_START(f) AF_TEST_ON_START_2(f, "")
#  else
#    define AF_TEST_ON_START(f) AF_TEST_ON_START_2(f, "_")
#  endif
#else
#  define AF_TEST_ON_START(f)                         \
    static void f(void) __attribute__((constructor)); \
    static void f(void)
#endif

#define AF_TEST_CONCAT4(a, b, c, d) a##b##c##d
#define AF_TEST_CONCAT3(a, b, c) AF_TEST_CONCAT4(a, b, _, c)

#define TEST(name)                                                   \
  static void AF_TEST_CONCAT3(af_test_run_, __LINE__,                \
                              AF_TEST_FILE)(int, af_test_report);    \
  AF_TEST_ON_START(                                                  \
      AF_TEST_CONCAT3(af_test_case_, __LINE__, AF_TEST_FILE)) {      \
    int n = af_tests_list.size;                                      \
    if (n < AF_TESTS_SIZE_LIMIT) {                                   \
      af_tests_list.size++;                                          \
      af_tests_list.tests[n].test_fn = AF_TEST_CONCAT3(              \
          af_test_run_, __LINE__, AF_TEST_FILE);                     \
      strcpy(af_tests_list.tests[n].test_name, name);                \
      af_tests_list.tests[n].assertions = 0;                         \
    }                                                                \
  }                                                                  \
  static void AF_TEST_CONCAT3(af_test_run_, __LINE__, AF_TEST_FILE)( \
      int af_test_index_, af_test_report af_test_report_)

#define REQUIRE(ok) \
  af_test_report_(af_test_index_, __FILE__, __LINE__, (ok))

#ifdef __cplusplus
}
#endif

#endif
