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
  msg__init ((Msg *) message);
  return message;
}

void
riemann_message_free (riemann_message_t *message)
{
  if (!message)
    return;

  free (message);
}

int
riemann_message_set_events_n (riemann_message_t *message,
                              size_t n_events,
                              riemann_event_t **events)
{
  if (!message)
    {
      errno = EINVAL;
      return -1;
    }

  if (n_events < 1)
    {
      errno = ERANGE;
      return -1;
    }

  if (!events)
    {
      errno = EINVAL;
      return -1;
    }

  message->n_events = n_events;
  message->events = events;
  return 0;
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
