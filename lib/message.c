/* riemann/message.c -- Riemann C client library
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

#include <riemann/message.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

riemann_message_t *
riemann_message_new (void)
{
  riemann_message_t *message;

  message = (riemann_message_t *)malloc (sizeof (riemann_message_t));
  msg__init (message);

  return message;
}

void
riemann_message_free (riemann_message_t *message)
{
  if (!message)
    return;

  msg__free_unpacked ((Msg *)message, NULL);
}

int
riemann_message_set_events_n (riemann_message_t *message,
                              size_t n_events,
                              riemann_event_t **events)
{
  if (!message)
    return -EINVAL;

  if (n_events < 1)
    return -ERANGE;

  if (!events)
    return -EINVAL;

  if (message->events)
    {
      size_t i;

      for (i = 0; i < message->n_events; i++)
        riemann_event_free (message->events[i]);

      free (message->events);
    }

  message->n_events = n_events;
  message->events = events;
  return 0;
}

static riemann_event_t **
_riemann_message_combine_events (riemann_event_t **events,
                                 riemann_event_t *event, size_t *n_events,
                                 va_list aq)
{
  size_t alloced;
  va_list ap;

  if (!events || !event || !n_events)
    {
      errno = EINVAL;
      return NULL;
    }
  alloced = *n_events;

  va_copy (ap, aq);

  do
    {
      if (*n_events >= alloced)
        {
          alloced *= 2;
          events = realloc (events, sizeof (riemann_event_t *) * alloced);
        }

      event = va_arg (ap, riemann_event_t *);
      events[*n_events] = event;
      if (event)
        (*n_events)++;
    }
  while (event != NULL);

  va_end (ap);

  return events;
}

int
riemann_message_set_events (riemann_message_t *message, ...)
{
  size_t n_events = 1;
  riemann_event_t **events, **nevents, *event;
  va_list ap;
  int result;

  if (!message)
    return -EINVAL;

  va_start (ap, message);
  event = va_arg (ap, riemann_event_t *);
  if (!event)
    {
      va_end (ap);
      return -ERANGE;
    }

  events = malloc (sizeof (riemann_event_t *));

  events[0] = event;
  nevents = _riemann_message_combine_events (events, events[0], &n_events, ap);
  va_end (ap);

  if (n_events == 0)
    {
      free (events);
      return -EINVAL;
    }

  result = riemann_message_set_events_n (message, n_events, nevents);

  if (result != 0)
    {
      free (events);
    }

  return result;
}

riemann_message_t *
riemann_message_create_with_events (riemann_event_t *event, ...)
{
  riemann_message_t *message;
  riemann_event_t **events;
  va_list ap;
  size_t n_events = 1;
  int result;

  if (!event)
    {
      errno = EINVAL;
      return NULL;
    }

  message = riemann_message_new ();
  if (!message)
    return NULL;

  events = malloc (sizeof (riemann_event_t *));
  events[0] = event;

  va_start (ap, event);
  events = _riemann_message_combine_events (events, event, &n_events, ap);
  va_end (ap);

  if (n_events == 0)
    {
      riemann_message_free (message);
      errno = EINVAL;
      return NULL;
    }

  result = riemann_message_set_events_n (message, n_events, events);

  if (result != 0)
    {
      riemann_message_free (message);
      errno = EINVAL;
      return NULL;
    }

  return message;
}


int
riemann_message_set_query (riemann_message_t *message,
                           riemann_query_t *query)
{
  if (!message || !query)
    return -EINVAL;

  if (message->query)
    riemann_query_free (message->query);
  message->query = query;

  return 0;
}

riemann_message_t *
riemann_message_create_with_query (riemann_query_t *query)
{
  riemann_message_t *message;
  int result;

  if (!query)
    {
      errno = EINVAL;
      return NULL;
    }

  message = riemann_message_new ();
  if (!message)
    return NULL;

  result = riemann_message_set_query (message, query);
  if (result != 0)
    {
      riemann_message_free (message);
      errno = -result;
      return NULL;
    }

  return message;
}

uint8_t *
riemann_message_to_buffer (riemann_message_t *message, size_t *len)
{
  size_t l;
  struct
  {
    uint32_t header;
    uint8_t data[0];
  } *buff;

  if (!message)
    {
      errno = EINVAL;
      return NULL;
    }

  l = msg__get_packed_size (message) + sizeof (buff->header);
  buff = malloc (l);
  msg__pack (message, buff->data);

  buff->header = htonl (l - sizeof (buff->header));

  if (len)
    *len = l;

  return (uint8_t *)buff;
}

riemann_message_t *
riemann_message_from_buffer (uint8_t *buffer, size_t len)
{
  if (!buffer || len == 0)
    return NULL;

  return msg__unpack (NULL, len, buffer);
}
