#include <riemann/simple.h>

START_TEST (test_riemann_simple_send)
{
  riemann_client_t *client;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);

  ck_assert_errno (riemann_send (NULL, RIEMANN_EVENT_FIELD_NONE), ENOTCONN);

  ck_assert_errno (riemann_send (client, 255), EPROTO);

  ck_assert_errno (riemann_send (client,
                                 RIEMANN_EVENT_FIELD_SERVICE, "test-simple",
                                 RIEMANN_EVENT_FIELD_STATE, "ok",
                                 RIEMANN_EVENT_FIELD_NONE),
                   0);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_simple_query)
{
  riemann_client_t *client;
  riemann_message_t *response;

  ck_assert (riemann_query (NULL, "service = \"test-simple\"") == NULL);
  ck_assert_errno (-errno, ENOTCONN);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "127.0.0.1", 5555);

  riemann_send (client,
                RIEMANN_EVENT_FIELD_SERVICE, "test-simple",
                RIEMANN_EVENT_FIELD_STATE, "ok",
                RIEMANN_EVENT_FIELD_NONE);

  response = riemann_query (client, "service = \"test-simple\"");

  ck_assert (response != NULL);
  ck_assert_int_eq (response->ok, 1);

  riemann_message_free (response);
  riemann_client_free (client);
}
END_TEST

static TCase *
test_riemann_simple (void)
{
  TCase *test_simple;

  test_simple = tcase_create ("Simple");

  if (network_tests_enabled ())
    {
      tcase_add_test (test_simple, test_riemann_simple_send);
      tcase_add_test (test_simple, test_riemann_simple_query);
    }

  return test_simple;
}
