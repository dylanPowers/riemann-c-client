#include <check.h>
#include <stdlib.h>

#include <riemann/client.h>

#include "config.h"

START_TEST (test_riemann_c_client_library)
{
  ck_assert_str_eq (riemann_client_version (), PACKAGE_VERSION);
  ck_assert_str_eq (riemann_client_version_string (), PACKAGE_STRING);
}
END_TEST

int
main (void)
{
  Suite *suite;
  SRunner *runner;
  TCase *test_library;

  int nfailed;

  suite = suite_create ("Riemann C client library tests");

  test_library = tcase_create ("Core");
  tcase_add_test (test_library, test_riemann_c_client_library);
  suite_add_tcase (suite, test_library);

  runner = srunner_create (suite);

  srunner_run_all (runner, CK_ENV);
  nfailed = srunner_ntests_failed (runner);
  srunner_free (runner);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
