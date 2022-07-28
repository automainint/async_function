#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct af_tests_list af_tests_list = { 0 };

static void report(ptrdiff_t i, bool ok) {
  if (!ok)
    af_tests_list.tests[i].test_status = false;
}

static long long ns_to_ms(long long ns) {
  return (ns + 500000) / 1000000;
}

static long long sec_to_ms(long long sec) {
  return 1000 * sec;
}

static bool is_term() {
  if (getenv("TERM") == NULL)
    return false;
  return true;
}

enum code_value { white, yellow, red, green };

static void code(bool term, int c) {
  if (term) {
    if (c == white)
      printf("\x1b[37m");
    if (c == yellow)
      printf("\x1b[33m");
    if (c == red)
      printf("\x1b[31m");
    if (c == green)
      printf("\x1b[32m");
  }
}

int main(int argc, char **argv) {
  int  status = 0;
  bool term   = is_term();

  for (ptrdiff_t i = 0; i < af_tests_list.size; i++) {
    code(term, yellow);
    printf("[ RUN... ] %s ", af_tests_list.tests[i].test_name);
    code(term, white);

    struct timespec begin, end;
    timespec_get(&begin, TIME_UTC);

    af_tests_list.tests[i].test_fn(i, report);

    timespec_get(&end, TIME_UTC);
    int duration = (int) (ns_to_ms(end.tv_nsec - begin.tv_nsec) +
                          sec_to_ms(end.tv_sec - begin.tv_sec));

    printf("\r");

    if (af_tests_list.tests[i].test_status == false) {
      code(term, red);
      printf("[ RUN    ] %s\n", af_tests_list.tests[i].test_name);
      printf("[ FAILED ] %s - %d ms\n",
             af_tests_list.tests[i].test_name, duration);
      code(term, white);
      status = 1;
    } else {
      code(term, green);
      printf("[ RUN    ] %s\n", af_tests_list.tests[i].test_name);
      printf("[     OK ] %s - %d ms\n",
             af_tests_list.tests[i].test_name, duration);
      code(term, white);
    }
  }

  return status;
}
