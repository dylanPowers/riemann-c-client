#ifndef __MADHOUSE_TESTS_H__
#define __MADHOUSE_TESTS_H__ 1

#include <check.h>

#define _ck_assert_float(X, O, Y) \
  ck_assert_msg((X) O (Y), \
                "Assertion '"#X#O#Y"' failed: "#X"==%f, "#Y"==%f", X, Y)

#define ck_assert_float_eq(X, Y) \
  _ck_assert_float(X, ==, Y)

#define ck_assert_errno(X, E) \
  ck_assert_msg((X) == -(E), \
                "Assertion '" #X " == -" #E "' failed: errno==%d (%s), " \
                "expected==%d (%s)", \
                errno, (char *)strerror (errno), E, (char *)strerror (E))

#endif
