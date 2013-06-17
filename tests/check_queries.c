#include <riemann/query.h>

START_TEST (test_riemann_query_static_init)
{
  riemann_query_t query = RIEMANN_QUERY_INIT;

  ck_assert (query.string == NULL);
}
END_TEST

START_TEST (test_riemann_query_new)
{
  riemann_query_t *query;

  query = riemann_query_new (NULL);
  ck_assert (query != NULL);
  riemann_query_free (query);

  query = riemann_query_new ("status = \"ok\"");
  ck_assert (query != NULL);
  ck_assert_str_eq (query->string, "status = \"ok\"");
  riemann_query_free (query);
}
END_TEST

START_TEST (test_riemann_query_set_string)
{
  riemann_query_t *query;

  query = riemann_query_new (NULL);
  ck_assert_errno (riemann_query_set_string (query, "status = \"ok\""), 0);
  ck_assert_str_eq (query->string, "status = \"ok\"");
  riemann_query_free (query);
}
END_TEST

static TCase *
test_riemann_queries (void)
{
  TCase *tests;

  tests = tcase_create ("Queries");
  tcase_add_test (tests, test_riemann_query_static_init);
  tcase_add_test (tests, test_riemann_query_new);
  tcase_add_test (tests, test_riemann_query_set_string);

  return tests;
}
