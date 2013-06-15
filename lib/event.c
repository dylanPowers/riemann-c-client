/* riemann/event.c -- Riemann C client library
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

#include <riemann/event.h>

#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

riemann_event_t *
riemann_event_init (riemann_event_t *event)
{
  if (!event)
    {
      errno = EINVAL;
      return NULL;
    }

  event__init((Event *) event);
  return event;
}

riemann_event_t *
riemann_event_new (void)
{
  riemann_event_t *event;

  event = (riemann_event_t *)malloc (sizeof (riemann_event_t));
  return riemann_event_init (event);
}

void
riemann_event_free (riemann_event_t *event)
{
  if (!event)
    return;

  if (event->state)
    free (event->state);
  if (event->service)
    free (event->service);
  if (event->host)
    free (event->host);
  if (event->description)
    free (event->description);

  free (event);
}

static void
_riemann_event_set_string (char **str, const char *value)
{
  if (*str)
    free (*str);
  *str = strdup (value);
}

int
riemann_event_set (riemann_event_t *event, ...)
{
  va_list ap;
  riemann_event_field_t field;

  if (!event)
    {
      errno = EINVAL;
      return -1;
    }

  va_start (ap, event);
  do
    {
      field = (riemann_event_field_t) va_arg (ap, riemann_event_field_t);

      switch (field)
        {
        case RIEMANN_EVENT_FIELD_NONE:
          break;

        case RIEMANN_EVENT_FIELD_TIME:
          event->time = (int64_t) va_arg (ap, int64_t);
          event->has_time = 1;
          break;

        case RIEMANN_EVENT_FIELD_STATE:
          _riemann_event_set_string (&event->state, va_arg (ap, char *));
          break;

        case RIEMANN_EVENT_FIELD_SERVICE:
          _riemann_event_set_string (&event->service, va_arg (ap, char *));
          break;

        case RIEMANN_EVENT_FIELD_HOST:
          _riemann_event_set_string (&event->host, va_arg (ap, char *));
          break;

        case RIEMANN_EVENT_FIELD_DESCRIPTION:
          _riemann_event_set_string (&event->description, va_arg (ap, char *));
          break;

        case RIEMANN_EVENT_FIELD_TAGS:
          va_end (ap);
          errno = ENOSYS;
          return -1;

        case RIEMANN_EVENT_FIELD_TTL:
          event->ttl = (float) va_arg (ap, double);
          event->has_ttl = 1;
          break;

        case RIEMANN_EVENT_FIELD_ATTRIBUTES:
          va_end (ap);
          errno = ENOSYS;
          return -1;

        case RIEMANN_EVENT_FIELD_METRIC_S64:
          event->metric_sint64 = va_arg (ap, int64_t);
          event->has_metric_sint64 = 1;
          break;

        case RIEMANN_EVENT_FIELD_METRIC_D:
          event->metric_d = va_arg (ap, double);
          event->has_metric_d = 1;
          break;

        case RIEMANN_EVENT_FIELD_METRIC_F:
          event->metric_f = (float) va_arg (ap, double);
          event->has_metric_f = 1;
          break;

        default:
          va_end (ap);
          errno = EPROTO;
          return -1;
        }
    }
  while (field != RIEMANN_EVENT_FIELD_NONE);

  return 0;
}
