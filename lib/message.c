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

static void
_riemann_message_free_events (riemann_message_t *message)
{
  size_t i;

  if (!message || !message->events)
    return;

  for (i = 0; i < message->n_events; i++)
    riemann_event_free (message->events[i]);

  message->n_events = 0;
  free (message->events);
  message->events = NULL;
}

void
riemann_message_free (riemann_message_t *message)
{
  if (!message)
    return;

  _riemann_message_free_events (message);

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

  _riemann_message_free_events (message);

  message->n_events = n_events;
  message->events = events;
  return 0;
}

static riemann_event_t **
_riemann_message_combine_events (riemann_event_t **events,
                                 riemann_event_t *event, size_t *n_events,
                                 va_list aq)
{
  size_t alloced = *n_events;
  va_list ap;

  if (!events || !event || !n_events)
    {
      errno = EINVAL;
      return NULL;
    }

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

  buff->header = htonl (l);

  if (len)
    *len = l;

  return (uint8_t *)buff;
}
