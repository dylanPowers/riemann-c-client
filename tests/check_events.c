#include <riemann/attribute.h>
#include <riemann/event.h>

START_TEST (test_riemann_event_new)
{
  riemann_event_t *event;

  event = riemann_event_new ();
  ck_assert (event != NULL);

  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_free)
{
  errno = 0;
  riemann_event_free (NULL);
  ck_assert_errno (-errno, EINVAL);
}
END_TEST

START_TEST (test_riemann_event_set)
{
  riemann_event_t *event;

  ck_assert_errno (riemann_event_set (NULL), EINVAL);

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

  ck_assert_errno (riemann_event_set (event, RIEMANN_EVENT_FIELD_TAGS,
                                      "tag-1", "tag-2", NULL,
                                      RIEMANN_EVENT_FIELD_NONE), 0);
  ck_assert_int_eq (event->n_tags, 2);
  ck_assert_str_eq (event->tags[0], "tag-1");
  ck_assert_str_eq (event->tags[1], "tag-2");

  ck_assert (riemann_event_set (event, RIEMANN_EVENT_FIELD_TTL, (float) 1,
                                RIEMANN_EVENT_FIELD_NONE) == 0);
  ck_assert_int_eq (event->has_ttl, 1);
  ck_assert_float_eq (event->ttl, (float) 1);

  ck_assert_errno
    (riemann_event_set (event, RIEMANN_EVENT_FIELD_ATTRIBUTES,
                        riemann_attribute_create ("key-1", "value-1"),
                        riemann_attribute_create ("key-2", "value-2"),
                        NULL,
                        RIEMANN_EVENT_FIELD_NONE), 0);
  ck_assert_int_eq (event->n_attributes, 2);
  ck_assert_str_eq (event->attributes[0]->key, "key-1");
  ck_assert_str_eq (event->attributes[1]->value, "value-2");

  ck_assert_errno
    (riemann_event_set (event, RIEMANN_EVENT_FIELD_ATTRIBUTES,
                        riemann_attribute_create ("key-3", "value-3"),
                        NULL,
                        RIEMANN_EVENT_FIELD_NONE), 0);
  ck_assert_int_eq (event->n_attributes, 1);
  ck_assert_str_eq (event->attributes[0]->key, "key-3");
  ck_assert_str_eq (event->attributes[0]->value, "value-3");

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

  ck_assert_errno (riemann_event_set (event, RIEMANN_EVENT_FIELD_METRIC_F * 2,
                                      0,
                                      RIEMANN_EVENT_FIELD_NONE), EPROTO);

  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_set_one)
{
  riemann_event_t *event;

  event = riemann_event_new ();

  ck_assert (riemann_event_set_one (event, TIME, (int64_t) 1234) == 0);
  ck_assert_int_eq (event->has_time, 1);
  ck_assert_int_eq (event->time, 1234);

  ck_assert (riemann_event_set_one (event, HOST, "localhost") == 0);
  ck_assert_str_eq (event->host, "localhost");

  ck_assert (riemann_event_set_one (event, TAGS, "tag-1", "tag-2", NULL) == 0);
  ck_assert_str_eq (event->tags[0], "tag-1");
  ck_assert_str_eq (event->tags[1], "tag-2");

  ck_assert (riemann_event_set_one (event, HOST, "localhost2") == 0);
  ck_assert_str_eq (event->host, "localhost2");

  ck_assert (riemann_event_set_one (event, TAGS, "tag-3", NULL) == 0);
  ck_assert_int_eq (event->n_tags, 1);
  ck_assert_str_eq (event->tags[0], "tag-3");

  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_create)
{
  riemann_event_t *event;

  event = riemann_event_create (RIEMANN_EVENT_EMPTY);
  ck_assert (event != NULL);
  riemann_event_free (event);

  event = riemann_event_create (RIEMANN_EVENT_FIELD_HOST, "localhost",
                                RIEMANN_EVENT_FIELD_SERVICE, "test",
                                RIEMANN_EVENT_FIELD_NONE);
  ck_assert (event != NULL);
  ck_assert_str_eq (event->host, "localhost");
  ck_assert_str_eq (event->service, "test");
  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_tag_add)
{
  riemann_event_t *event;

  event = riemann_event_new ();

  ck_assert_errno (riemann_event_tag_add (NULL, NULL), EINVAL);
  ck_assert_errno (riemann_event_tag_add (event, NULL), EINVAL);

  ck_assert_errno (riemann_event_tag_add (event, "test-tag"), 0);

  ck_assert_int_eq (event->n_tags, 1);
  ck_assert_str_eq (event->tags[0], "test-tag");

  riemann_event_free (event);
}
END_TEST

START_TEST (test_riemann_event_attribute_add)
{
  riemann_event_t *event;

  event = riemann_event_new ();

  ck_assert_errno (riemann_event_attribute_add (NULL, NULL), EINVAL);
  ck_assert_errno (riemann_event_attribute_add (event, NULL), EINVAL);

  ck_assert_errno (riemann_event_attribute_add
                   (event, riemann_attribute_create ("test-key", "value")), 0);

  ck_assert_int_eq (event->n_attributes, 1);
  ck_assert_str_eq (event->attributes[0]->key, "test-key");

  riemann_event_free (event);
}
END_TEST

static TCase *
test_riemann_events (void)
{
  TCase *test_events;

  test_events = tcase_create ("Events");
  tcase_add_test (test_events, test_riemann_event_new);
  tcase_add_test (test_events, test_riemann_event_free);
  tcase_add_test (test_events, test_riemann_event_set);
  tcase_add_test (test_events, test_riemann_event_set_one);
  tcase_add_test (test_events, test_riemann_event_tag_add);
  tcase_add_test (test_events, test_riemann_event_attribute_add);
  tcase_add_test (test_events, test_riemann_event_create);

  return test_events;
}
