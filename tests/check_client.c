#include <riemann/client.h>
#include <stdlib.h>

static int
network_tests_enabled (void)
{
  char *env;

  env = getenv ("RIEMANN_TESTS_NETWORK");
  if (!env)
    return 0;

  if (env[0] == 'y' || env[0] == 'Y' || env[0] == '1' ||
      (env[0] == 'o' && env[1] == 'n'))
    return 1;

  return 0;
}

START_TEST (test_riemann_client_new)
{
  riemann_client_t *client;

  client = riemann_client_new ();
  ck_assert (client != NULL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_connect)
{
  riemann_client_t *client;

  client = riemann_client_new ();

  ck_assert_errno (riemann_client_connect (NULL, RIEMANN_CLIENT_TCP,
                                           "localhost", 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_NONE,
                                           "localhost", 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                           NULL, 5555), EINVAL);
  ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                           "localhost", -1), ERANGE);

  if (network_tests_enabled ())
    {
      ck_assert (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                         "localhost", 5555) == 0);
      ck_assert_errno (riemann_client_disconnect (client), 0);

      ck_assert_errno (riemann_client_connect (client, RIEMANN_CLIENT_TCP,
                                               "non-existent.example.com", 5555),
                       EADDRNOTAVAIL);
    }

  ck_assert (client != NULL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_create)
{
  riemann_client_t *client;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  ck_assert (client != NULL);
  ck_assert_errno (riemann_client_disconnect (client), 0);
  ck_assert (client != NULL);

  riemann_client_free (client);
}
END_TEST

START_TEST (test_riemann_client_send_message)
{
  riemann_client_t *client, *client_fresh;
  riemann_message_t *message;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  ck_assert_errno (riemann_client_send_message (NULL, message), ENOTCONN);
  ck_assert_errno (riemann_client_send_message (client, NULL), EINVAL);

  client_fresh = riemann_client_new ();
  ck_assert_errno (riemann_client_send_message (client_fresh, message), ENOTCONN);
  riemann_client_free (client_fresh);

  ck_assert_errno (riemann_client_send_message (client, message), 0);

  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_UDP, "localhost", 5555);
  ck_assert_errno (riemann_client_send_message (client, message), 0);

  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_client_recv_message)
{
  riemann_client_t *client, *client_fresh;
  riemann_message_t *message, *response = NULL;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  message = riemann_message_create_with_events
    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                           RIEMANN_EVENT_FIELD_STATE, "ok",
                           RIEMANN_EVENT_FIELD_NONE),
     NULL);

  client_fresh = riemann_client_new ();
  ck_assert (riemann_client_recv_message (client_fresh) == NULL);
  ck_assert_errno (-errno, ENOTCONN);
  riemann_client_free (client_fresh);

  riemann_client_send_message (client, message);
  ck_assert ((response = riemann_client_recv_message (client)) != NULL);
  ck_assert_int_eq (response->ok, 1);
  riemann_message_free (response);

  riemann_client_free (client);

  client = riemann_client_create (RIEMANN_CLIENT_UDP, "localhost", 5555);

  ck_assert (riemann_client_recv_message (client) == NULL);
  ck_assert_errno (-errno, ENOTSUP);

  riemann_client_free (client);

  riemann_message_free (message);
}
END_TEST

START_TEST (test_riemann_client_send_message_oneshot)
{
  riemann_client_t *client, *client_fresh;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  ck_assert_errno (riemann_client_send_message_oneshot
                   (NULL, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), ENOTCONN);
  ck_assert_errno (riemann_client_send_message (client, NULL), EINVAL);

  client_fresh = riemann_client_new ();
  ck_assert_errno (riemann_client_send_message_oneshot
                   (client_fresh, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), ENOTCONN);
  riemann_client_free (client_fresh);

  ck_assert_errno (riemann_client_send_message_oneshot
                   (client, riemann_message_create_with_events
                    (riemann_event_create (RIEMANN_EVENT_FIELD_SERVICE, "test",
                                           RIEMANN_EVENT_FIELD_STATE, "ok",
                                           RIEMANN_EVENT_FIELD_NONE),
                     NULL)), 0);

  riemann_client_free (client);
}
END_TEST

static TCase *
test_riemann_client (void)
{
  TCase *test_client;

  test_client = tcase_create ("Client");
  tcase_add_test (test_client, test_riemann_client_new);
  tcase_add_test (test_client, test_riemann_client_connect);

  if (network_tests_enabled ())
    {
      tcase_add_test (test_client, test_riemann_client_create);
      tcase_add_test (test_client, test_riemann_client_send_message);
      tcase_add_test (test_client, test_riemann_client_send_message_oneshot);
      tcase_add_test (test_client, test_riemann_client_recv_message);
    }

  return test_client;
}
