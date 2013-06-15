#include <check.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <riemann/event.h>

#include "config.h"

#define _ck_assert_float(X, O, Y) ck_assert_msg((X) O (Y), "Assertion '"#X#O#Y"' failed: "#X"==%f, "#Y"==%f", X, Y)
#define ck_assert_float_eq(X, Y) _ck_assert_float(X, ==, Y)

#define ck_assert_errno(E) ck_assert_msg(errno == (E), "Assertion 'errno == "#E"' failed: errno==%d (%s), expected==%d (%s)", errno, (char *)strerror (errno), E, (char *)strerror (E))

START_TEST (test_riemann_event_init)
{
  riemann_event_t event;

  ck_assert (riemann_event_init (&event) != NULL);
}
END_TEST

START_TEST (test_riemann_event_new)
{
  riemann_event_t *event;

  event = riemann_event_new ();
  ck_assert (event != NULL);

  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_set)
{
  riemann_event_t *event;

  event = riemann_event_new ();
  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_NONE) == 0);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_TIME, (int64_t) 1234,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_time, 1);
  ck_assert_int_eq (event->time, 1234);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_STATE, "ok",
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert (event->state != NULL);
  ck_assert_str_eq (event->state, "ok");

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_SERVICE, "test",
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert (event->service != NULL);
  ck_assert_str_eq (event->service, "test");

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_HOST, "localhost",
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert (event->host != NULL);
  ck_assert_str_eq (event->host, "localhost");

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_DESCRIPTION, "something",
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert (event->description != NULL);
  ck_assert_str_eq (event->description, "something");

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_TAGS,
                                "tag-1", "tag-2",
                                RIEMANN_EVENT_FIELD_NONE) == -1);
  ck_assert_errno (ENOSYS);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_TTL, (float) 1,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_ttl, 1);
  ck_assert_float_eq (event->ttl, (float) 1);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_ATTRIBUTES,
                                "key-1", "value-1",
                                "key-2", "value-2",
                                NULL,
                                RIEMANN_EVENT_FIELD_NONE) == -1);
  ck_assert_errno (ENOSYS);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_METRIC_S64,
                                (int64_t) 12345,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_metric_sint64, 1);
  ck_assert_int_eq (event->metric_sint64, 12345);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_METRIC_D,
                                (double) 1.5,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_metric_d, 1);
  ck_assert_float_eq (event->metric_d, 1.5);

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_METRIC_F,
                                (float) 1.5,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_metric_f, 1);
  ck_assert_float_eq (event->metric_f, 1.5);

  riemann_event_free (event);
}
END_TEST

int
main (void)
{
  Suite *suite;
  SRunner *runner;
  TCase *test_events;

  int nfailed;

  suite = suite_create ("Riemann C client library tests");

  test_events = tcase_create ("Events");
  tcase_add_test (test_events, test_riemann_event_init);
  tcase_add_test (test_events, test_riemann_event_new);
  tcase_add_test (test_events, test_riemann_event_set);
  suite_add_tcase (suite, test_events);

  runner = srunner_create (suite);

  srunner_run_all (runner, CK_ENV);
  nfailed = srunner_ntests_failed (runner);
  srunner_free (runner);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
