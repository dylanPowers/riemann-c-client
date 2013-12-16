/* riemann/simple.c -- Riemann C client library
 * Copyright (C) 2013  Gergely Nagy <algernon@madhouse-project.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <riemann/simple.h>

#include "riemann/_private.h"

int
riemann_send (riemann_client_t *client,
              riemann_event_field_t field, ...)
{
  riemann_message_t *message;
  riemann_event_t *event;
  va_list ap;
  int e;

  if (!client)
    return -ENOTCONN;

  event = riemann_event_new ();
  if (!event)
    return -errno;

  va_start (ap, field);
  if ((e = riemann_event_set_va (event, field, ap)) != 0)
    {
      va_end (ap);
      riemann_event_free (event);

      return e;
    }
  va_end (ap);

  message = riemann_message_create_with_events (event, NULL);
  if (!message)
    {
      e = errno;

      riemann_event_free (event);
      return -e;
    }

  return riemann_client_send_message_oneshot (client, message);
}

riemann_message_t *
riemann_query (riemann_client_t *client, const char *query)
{
  int e;
  riemann_message_t *response;

  e = riemann_client_send_message_oneshot
    (client, riemann_message_create_with_query (riemann_query_new (query)));
  if (e != 0)
    {
      errno = e;
      return NULL;
    }

  return riemann_client_recv_message (client);
}
