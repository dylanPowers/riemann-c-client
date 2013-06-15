#include <check.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <riemann/message.h>
#include <riemann/event.h>

#define ck_assert_errno(X, E) ck_assert_msg((X) == -(E), "Assertion '"#X" == -"#E"' failed: errno==%d (%s), expected==%d (%s)", errno, (char *)strerror (errno), E, (char *)strerror (E))

START_TEST (test_riemann_message_new)
{
  riemann_message_t *message;

  message = riemann_message_new ();
  ck_assert (message != NULL);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_message_set_events_n)
{
  riemann_message_t *message;
  riemann_event_t *event1, *event2, **events;

  ck_assert_errno (riemann_message_set_events_n (NULL, 0, NULL), EINVAL);

  message = riemann_message_new ();

  ck_assert_errno (riemann_message_set_events_n (message, 0, NULL), ERANGE);
  ck_assert_errno (riemann_message_set_events_n (message, 1, NULL), EINVAL);

  /* --- */

  event1 = riemann_event_new ();
  event2 = riemann_event_new ();

  riemann_event_set (event1,
                     RIEMANN_EVENT_FIELD_HOST, "localhost",
                     RIEMANN_EVENT_FIELD_STATE, "ok",
                     RIEMANN_EVENT_FIELD_NONE);
  riemann_event_set (event2,
                     RIEMANN_EVENT_FIELD_HOST, "localhost",
                     RIEMANN_EVENT_FIELD_SERVICE, "test",
                     RIEMANN_EVENT_FIELD_STATE, "failed",
                     RIEMANN_EVENT_FIELD_NONE);

  events = malloc (sizeof (riemann_event_t *) * 3);
  events[0] = event1;
  events[1] = event2;

  ck_assert (riemann_message_set_events_n (message, 2, events) == 0);
  ck_assert_str_eq (message->events[0]->host, "localhost");
  ck_assert_str_eq (message->events[0]->state, "ok");
  ck_assert_str_eq (message->events[1]->service, "test");
  ck_assert_str_eq (message->events[1]->state, "failed");

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_message_to_buffer)
{
  riemann_message_t *message;
  riemann_event_t **events;
  uint8_t *buffer;
  size_t len;

  events = malloc (sizeof (riemann_event_t *));

  message = riemann_message_new ();
  events[0] = riemann_event_new ();
  riemann_event_set_one (events[0], SERVICE, "test");
  riemann_message_set_events_n (message, 1, events);

  ck_assert (riemann_message_to_buffer (NULL, NULL) == NULL);
  ck_assert_errno (-errno, EINVAL);

  ck_assert ((buffer = riemann_message_to_buffer (message, NULL)) != NULL);
  free (buffer);

  ck_assert ((buffer = riemann_message_to_buffer (message, &len)) != NULL);
  ck_assert_int_eq (len, 12);
  free (buffer);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_message_set_events)
{
  riemann_message_t *message;
  int result;

  ck_assert_errno (riemann_message_set_events (NULL), EINVAL);

  message = riemann_message_new ();

  ck_assert_errno (riemann_message_set_events (message, NULL), ERANGE);

  result = riemann_message_set_events
    (message,
     riemann_event_create (RIEMANN_EVENT_FIELD_HOST, "localhost",
                           RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     riemann_event_create (RIEMANN_EVENT_FIELD_HOST, "localhost",
                           RIEMANN_EVENT_FIELD_SERVICE, "test-two",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);
  ck_assert_int_eq (result, 0);
  ck_assert_str_eq (message->events[0]->service, "test");
  ck_assert_str_eq (message->events[1]->service, "test-two");

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_message_create_with_events)
{
  riemann_message_t *message;

  ck_assert (riemann_message_create_with_events (NULL) == NULL);
  ck_assert_errno (-errno, EINVAL);

  ck_assert ((message = riemann_message_create_with_events
              (riemann_event_create (RIEMANN_EVENT_FIELD_HOST, "localhost",
                                     RIEMANN_EVENT_FIELD_SERVICE, "test",
                                     RIEMANN_EVENT_FIELD_NONE),
               riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test-two",
                                     RIEMANN_EVENT_FIELD_NONE),
               NULL)) != NULL);
  ck_assert_str_eq (message->events[0]->service, "test");
  ck_assert_str_eq (message->events[1]->service, "test-two");

  riemann_message_free (message);
}
END_TEST

int
main (void)
{
  Suite *suite;
  SRunner *runner;
  TCase *test_messages;

  int nfailed;

  suite = suite_create ("Riemann C client library tests");

  test_messages = tcase_create ("Messages");
  tcase_add_test (test_messages, test_riemann_message_new);
  tcase_add_test (test_messages, test_riemann_message_set_events_n);
  tcase_add_test (test_messages, test_riemann_message_to_buffer);
  tcase_add_test (test_messages, test_riemann_message_set_events);
  tcase_add_test (test_messages, test_riemann_message_create_with_events);
  suite_add_tcase (suite, test_messages);

  runner = srunner_create (suite);

  srunner_run_all (runner, CK_ENV);
  nfailed = srunner_ntests_failed (runner);
  srunner_free (runner);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
