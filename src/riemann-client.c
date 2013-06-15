/* riemann/riemann-client.c -- Riemann C client library
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
#include <riemann/message.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

int
main (int argc, char *argv[])
{
  int c;
  riemann_event_t *event;
  riemann_message_t *message;
  uint8_t *data;
  size_t len;

  event = riemann_event_new ();

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
        {"state", required_argument, NULL, 's'},
        {"service", required_argument, NULL, 'S'},
        {"host", required_argument, NULL, 'h'},
        {"description", required_argument, NULL, 'D'},
        {"metric-sint64", required_argument, NULL, 'i'},
        {"metric-d", required_argument, NULL, 'd'},
        {"metric-f", required_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "s:S:h:D:i:d:f:",
                       long_options, &option_index);

      if (c == -1)
        break;

      switch (c)
        {
        case 's':
          riemann_event_set_one (event, STATE, optarg);
          break;

        case 'S':
          riemann_event_set_one (event, SERVICE, optarg);
          break;

        case 'h':
          riemann_event_set_one (event, HOST, optarg);
          break;

        case 'D':
          riemann_event_set_one (event, DESCRIPTION, optarg);
          break;

        case 'i':
          riemann_event_set_one (event, METRIC_S64, (int64_t) atoll (optarg));
          break;

        case 'd':
          riemann_event_set_one (event, METRIC_D, (double) atof (optarg));
          break;

        case 'f':
          riemann_event_set_one (event, METRIC_F, (float) atof (optarg));
          break;

        default:
          printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

  message = riemann_message_create_with_events (event, NULL);
  data = riemann_message_to_buffer (message, &len);

  write (1, data, len);

  return 0;
}
