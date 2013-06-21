/* riemann/riemann-send-events.c -- Riemann C client library
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

#include <riemann/riemann-client.h>

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "config.h"

static void
display_help ()
{
  printf (PACKAGE_NAME ", version " PACKAGE_VERSION "\n"
          "Usage: riemann-send-events [options...] [HOST] [PORT]\n"
          "\n"
          "Options:\n"
          " -s, --state=STATE                 Set the state of the event.\n"
          " -S, --service=SERVICE             Set the service sending the event.\n"
          " -h, --host=HOST                   Set the origin host of the event.\n"
          " -D, --description=DESCRIPTION     Set the description of the event.\n"
          " -i, --metric-sint64=METRIC        Set the 64bit integer metric of the event.\n"
          " -d, --metric-d=METRIC             Set the double metric of the event.\n"
          " -f, --metric-f=METRIC             Set the float metric of the event.\n"
          "\n"
          " -T, --tcp                         Send the message over TCP (default).\n"
          " -U, --udp                         Send the message over UDP.\n"
          " -?, --help                        This help screen.\n"
          "\n"
          "The HOST and PORT arguments are optional, and they default to\n"
          "\"localhost\" and 5555, respectively.\n"
          "\n"
          "Report " PACKAGE_NAME " bugs to " PACKAGE_BUGREPORT "\n");
}

int
main (int argc, char *argv[])
{
  riemann_event_t *event;
  riemann_message_t *response;
  riemann_client_t *client;
  riemann_client_type_t client_type = RIEMANN_CLIENT_TCP;
  char *host = "localhost";
  int port = 5555, c, e, exit_status = EXIT_SUCCESS;

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
        {"tcp", no_argument, NULL, 'T'},
        {"udp", no_argument, NULL, 'U'},
        {"help", no_argument, NULL, '?'},
        {NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "s:S:h:D:i:d:f:?UT",
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

        case 'T':
          client_type = RIEMANN_CLIENT_TCP;
          break;

        case 'U':
          client_type = RIEMANN_CLIENT_UDP;
          break;

        case '?':
          display_help ();
          exit (EXIT_SUCCESS);

        default:
          fprintf(stderr, "Unknown option: %c\n", c);
          exit (EXIT_FAILURE);
        }
    }

  riemann_event_set_one (event, TAGS, "riemann-c-client", "example:send-events",
                         NULL);
  riemann_event_set_one (event, ATTRIBUTES,
                         riemann_attribute_create ("x-client", "riemann-c-client"),
                         NULL);

  if (optind < argc)
    {
      host = argv[optind];

      if (optind + 1 < argc)
        port = atoi (argv[optind + 1]);
    }

  if (argc - optind > 2)
    {
      fprintf (stderr, "Too many arguments!\n");
      display_help ();
      exit (EXIT_FAILURE);
    }

  client = riemann_client_create (client_type, host, port);
  if (!client)
    {
      fprintf (stderr, "Unable to connect: %s\n", (char *)strerror (errno));
      exit_status = EXIT_FAILURE;
      goto end;
    }

  e = riemann_client_send_message_oneshot
    (client, riemann_message_create_with_events (event, NULL));
  if (e != 0)
    {
      fprintf (stderr, "Error sending message: %s\n", (char *)strerror (-e));
      exit_status = EXIT_FAILURE;
      goto end;
    }

  if (client_type == RIEMANN_CLIENT_UDP)
    goto end;

  response = riemann_client_recv_message (client);
  if (!response)
    {
      fprintf (stderr, "Error when asking for a message receipt: %s\n",
               strerror (errno));
      exit_status = EXIT_FAILURE;
      goto end;
    }

  if (response->ok != 1)
    {
      fprintf (stderr, "Message receipt failed: %s\n", response->error);
      exit_status = EXIT_FAILURE;
    }

  riemann_message_free (response);

 end:
  riemann_client_free (client);

  return exit_status;
}
