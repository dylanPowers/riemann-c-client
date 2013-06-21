/* riemann/riemann-query.c -- Riemann C client library
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
#include <time.h>

#include "config.h"

static void
display_help ()
{
  printf (PACKAGE_NAME ", version " PACKAGE_VERSION "\n"
          "Usage: riemann-query [OPTIONS...] QUERY [HOST] [PORT]\n"
          "\n"
          "Options:\n"
          " -?, --help                        This help screen.\n"
          "\n"
          "The HOST and PORT arguments are optional, and they default to\n"
          "\"localhost\" and 5555, respectively.\n"
          "\n"
          "Report " PACKAGE_NAME " bugs to " PACKAGE_BUGREPORT "\n");
}

static void
dump_event (size_t n, const riemann_event_t *event)
{
  size_t i;
  time_t t = event->time;

  printf ("Event #%zu:\n"
          "  time  = %lu - %s"
          "  state = %s\n"
          "  service = %s\n"
          "  host = %s\n"
          "  description = %s\n"
          "  ttl = %f\n"
          "  metric_sint64 = %lu\n"
          "  metric_d = %f\n"
          "  metric_f = %f\n",
          n,
          event->time, ctime (&t),
          event->state, event->service, event->host,
          event->description, event->ttl,
          event->metric_sint64, event->metric_d, event->metric_f);

  if (event->tags)
    {
      printf ("  tags = [ ");

      for (i = 0; i < event->n_tags; i++)
        printf ("%s ", event->tags[i]);

      printf ("]\n");
    }

  if (event->attributes)
    {
      printf ("  attributes = {\n");

      for (i = 0; i < event->n_attributes; i++)
        printf ("    %s = %s\n", event->attributes[i]->key,
                event->attributes[i]->value);

      printf ("  }\n");
    }

  printf ("\n");
}

int
main (int argc, char *argv[])
{
  riemann_message_t *response;
  riemann_client_t *client;
  char *host = "localhost", *query_string = NULL;
  int port = 5555, c, e, exit_status = EXIT_SUCCESS;
  size_t i;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
        {"help", no_argument, NULL, '?'},
        {NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "?",
                       long_options, &option_index);

      if (c == -1)
        break;

      switch (c)
        {
        case '?':
          display_help ();
          exit (EXIT_SUCCESS);

        default:
          fprintf(stderr, "Unknown option: %c\n", c);
          exit (EXIT_FAILURE);
        }
    }

  if (optind >= argc)
    {
      fprintf (stderr, "The QUERY argument is mandatory!\n");
      display_help ();
      exit (EXIT_FAILURE);
    }

  query_string = argv[optind];

  optind++;

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

  client = riemann_client_create (RIEMANN_CLIENT_TCP, host, port);
  if (!client)
    {
      fprintf (stderr, "Unable to connect: %s\n", (char *)strerror (errno));
      exit_status = EXIT_FAILURE;
      goto end;
    }

  e = riemann_client_send_message_oneshot
    (client, riemann_message_create_with_query (riemann_query_new (query_string)));
  if (e != 0)
    {
      fprintf (stderr, "Error sending message: %s\n", (char *)strerror (-e));
      exit_status = EXIT_FAILURE;
      goto end;
    }

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

  for (i = 0; i < response->n_events; i++)
    dump_event (i, response->events[i]);

  riemann_message_free (response);

 end:
  riemann_client_free (client);

  return exit_status;
}
