#include <check.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "tests.h"

#include "check_library.c"
#include "check_events.c"
#include "check_messages.c"

int
main (void)
{
  Suite *suite;
  SRunner *runner;

  int nfailed;

  suite = suite_create ("Riemann C client library tests");

  suite_add_tcase (suite, test_riemann_library ());
  suite_add_tcase (suite, test_riemann_events ());
  suite_add_tcase (suite, test_riemann_messages ());

  runner = srunner_create (suite);

  srunner_run_all (runner, CK_ENV);
  nfailed = srunner_ntests_failed (runner);
  srunner_free (runner);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
