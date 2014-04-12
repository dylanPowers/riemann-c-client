/* riemann/riemann-client.c -- Riemann C client library
 * Copyright (C) 2013, 2014  Gergely Nagy <algernon@madhouse-project.org>
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

#include "riemann/platform.h"

#if HAVE_JSON_C
#include <json.h>
#endif

static void
version_display (void)
{
  printf ("%s\n", riemann_client_version_string ());
}

static void
help_display (const char *app_name, void (*contents)(void))
{
  version_display ();
  printf ("Usage: %s COMMAND [options...] [HOST] [PORT]\n"
          "\n", app_name);
  contents();
  printf ("\n"
          "The HOST and PORT arguments are optional, and they default to\n"
          "\"localhost\" and 5555, respectively.\n"
          "\n"
          "Report " PACKAGE_NAME " bugs to " PACKAGE_BUGREPORT "\n");
}

static void
help_send (void)
{
  printf ("Sending events (send command):\n"
          "==============================\n"
          "\n"
          " Options:\n"
          "  -s, --state=STATE                 Set the state of the event.\n"
          "  -S, --service=SERVICE             Set the service sending the event.\n"
          "  -h, --host=HOST                   Set the origin host of the event.\n"
          "  -D, --description=DESCRIPTION     Set the description of the event.\n"
          "  -a, --attribute=KEY=VALUE         Add a new attribute to the event.\n"
          "  -t, --tag=TAG                     Add a tag to the event.\n"
          "  -i, --metric-sint64=METRIC        Set the 64bit integer metric of the event.\n"
          "  -d, --metric-d=METRIC             Set the double metric of the event.\n"
          "  -f, --metric-f=METRIC             Set the float metric of the event.\n"
          "  -L, --ttl=TTL                     Set the TTL of the event.\n"
          "\n"
          "  -T, --tcp                         Send the message over TCP (default).\n"
          "  -U, --udp                         Send the message over UDP.\n"
          "  -?, --help                        This help screen.\n");
}

static int
client_send (int argc, char *argv[])
{
  riemann_event_t *event;
  riemann_message_t *response;
  riemann_client_t *client;
  riemann_client_type_t client_type = RIEMANN_CLIENT_TCP;
  char *host = "localhost";
  int port = 5555, c, e, exit_status = EXIT_SUCCESS;

  event = riemann_event_new ();

  riemann_event_set_one (event, TAGS, "riemann-c-client", "example:send-events",
                         NULL);
  riemann_event_set_one (event, ATTRIBUTES,
                         riemann_attribute_create ("x-client", "riemann-c-client"),
                         NULL);

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
        {"state", required_argument, NULL, 's'},
        {"service", required_argument, NULL, 'S'},
        {"host", required_argument, NULL, 'h'},
        {"description", required_argument, NULL, 'D'},
        {"tag", required_argument, NULL, 't'},
        {"attribute", required_argument, NULL, 'a'},
        {"metric-sint64", required_argument, NULL, 'i'},
        {"metric-d", required_argument, NULL, 'd'},
        {"metric-f", required_argument, NULL, 'f'},
        {"tcp", no_argument, NULL, 'T'},
        {"udp", no_argument, NULL, 'U'},
        {"ttl", required_argument, NULL, 'L'},
        {"help", no_argument, NULL, '?'},
        {NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "s:S:h:D:a:t:i:d:f:?UTL:",
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

        case 'a':
          {
            char *key = optarg, *value;

            value = strchr (optarg, '=');
            if (!value)
              {
                fprintf (stderr, "Invalid attribute specification: %s\n", optarg);
                return EXIT_FAILURE;
              }
            value[0] = '\0';
            value++;

            riemann_event_attribute_add
              (event, riemann_attribute_create (key, value));

            break;
          }

        case 't':
          riemann_event_tag_add (event, optarg);
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

        case 'L':
          riemann_event_set_one (event, TTL, (float) atof (optarg));
          break;

        case '?':
          help_display (argv[0], help_send);
          return EXIT_SUCCESS;

        default:
          fprintf(stderr, "Unknown option: %c\n", c);
          help_display (argv[0], help_send);
          return EXIT_FAILURE;
        }
    }

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
      help_display (argv[0], help_send);
      return EXIT_FAILURE;
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

static void
query_dump_event (size_t n, const riemann_event_t *event)
{
  size_t i;
  time_t t = event->time;

  printf ("Event #%zu:\n"
          "  time  = %" PRId64 " - %s"
          "  state = %s\n"
          "  service = %s\n"
          "  host = %s\n"
          "  description = %s\n"
          "  ttl = %f\n",
          n,
          event->time, ctime (&t),
          event->state, event->service, event->host,
          event->description, event->ttl);

  if (event->has_metric_sint64)
    printf ("  metric_sint64 = %" PRId64 "\n", event->metric_sint64);
  if (event->has_metric_d)
    printf ("  metric_d = %f\n", event->metric_d);
  if (event->has_metric_f)
    printf ("  metric_f = %f\n", event->metric_f);

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

static void
query_dump_events (size_t n, const riemann_event_t **events)
{
  size_t i;

  for (i = 0; i < n; i++)
    query_dump_event (i, events[i]);
}

#if HAVE_JSON_C
static json_object *
query_dump_event_json (size_t __attribute__((unused)) n, const riemann_event_t *event)
{
  json_object *o;
  size_t i;

  o = json_object_new_object ();

  if (event->has_time)
    json_object_object_add (o, "time", json_object_new_int64 (event->time));
  if (event->state)
    json_object_object_add (o, "state", json_object_new_string (event->state));
  if (event->service)
    json_object_object_add (o, "service",
                            json_object_new_string (event->service));
  if (event->host)
    json_object_object_add (o, "host", json_object_new_string (event->host));
  if (event->description)
    json_object_object_add (o, "description",
                            json_object_new_string (event->description));
  if (event->has_ttl)
    json_object_object_add (o, "ttl", json_object_new_double (event->ttl));
  if (event->has_metric_sint64)
    json_object_object_add (o, "metric_sint64",
                            json_object_new_int64 (event->metric_sint64));
  if (event->has_metric_d)
    json_object_object_add (o, "metric_d",
                            json_object_new_double (event->metric_d));
  if (event->has_metric_f)
    json_object_object_add (o, "metric_f",
                            json_object_new_double (event->metric_f));

  if (event->tags)
    {
      json_object *tags;

      tags = json_object_new_array ();

      for (i = 0; i < event->n_tags; i++)
        json_object_array_add (tags, json_object_new_string (event->tags[i]));

      json_object_object_add (o, "tags", tags);
    }

  if (event->attributes)
    {
      json_object *attrs;

      attrs = json_object_new_object ();

      for (i = 0; i < event->n_attributes; i++)
        json_object_object_add (attrs, event->attributes[i]->key,
                                json_object_new_string (event->attributes[i]->value));

      json_object_object_add (o, "attributes", attrs);
    }

  return o;
}

static void
query_dump_events_json (size_t n, const riemann_event_t **events)
{
  size_t i;
  json_object *o;

  o = json_object_new_array ();

  for (i = 0; i < n; i++)
    json_object_array_add (o, query_dump_event_json (i, events[i]));

  printf ("%s\n", json_object_to_json_string_ext (o, JSON_C_TO_STRING_PLAIN));

  json_object_put (o);
}
#else
static void
query_dump_events_json (size_t __attribute__((unused)) n,
                        const riemann_event_t __attribute__((unused)) **events)
{
  fprintf (stderr, "JSON support not available in this build!\n");
  exit (EXIT_FAILURE);
}
#endif

static void
help_query (void)
{
  printf ("Querying events (query command):\n"
          "================================\n"
          "\n"
          "When using the query command, the QUERY argument must immediately follow\n"
          "the command on the command-line:\n"
          " riemann-client query QUERY [HOST] [PORT]\n"
          "\n"
          " Options:\n"
          "  -j, --json                        Output the results as a JSON array.\n"
          "  -?, --help                        This help screen.\n");
}

typedef void (*query_func_t) (size_t, const riemann_event_t **);

static int
client_query (int argc, char *argv[])
{
  riemann_message_t *response;
  riemann_client_t *client;
  char *host = "localhost", *query_string = NULL;
  int port = 5555, c, e, exit_status = EXIT_SUCCESS;
  query_func_t dump = query_dump_events;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
        {"help", no_argument, NULL, '?'},
        {"json", no_argument, NULL, 'j'},
        {NULL, 0, NULL, 0}
      };

      c = getopt_long (argc, argv, "?j",
                       long_options, &option_index);

      if (c == -1)
        break;

      switch (c)
        {
        case 'j':
          dump = query_dump_events_json;
          break;

        case '?':
          help_display (argv[0], help_query);
          return EXIT_SUCCESS;

        default:
          fprintf(stderr, "Unknown option: %c\n", c);
          help_display (argv[0], help_query);
          return EXIT_FAILURE;
        }
    }

  optind++;
  if (optind >= argc)
    {
      fprintf (stderr, "The QUERY argument is mandatory!\n");
      help_display (argv[0], help_query);
      return EXIT_FAILURE;
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
      help_display (argv[0], help_query);
      return EXIT_FAILURE;
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

  dump (response->n_events, (const riemann_event_t **)response->events);

  riemann_message_free (response);

 end:
  riemann_client_free (client);

  return exit_status;
}

static void
help_generic (void)
{
  printf ("Available commands: send and query.\n"
          "\n");
  help_send ();
  printf ("\n");
  help_query ();
}

int
main (int argc, char *argv[])
{
  const char *command = NULL;

  if (argc == 1)
    {
      help_display (argv[0], help_generic);
      exit (EXIT_SUCCESS);
    }

  if (argc < 2)
    {
      fprintf (stderr, "Not enough arguments!\n");
      help_display (argv[0], help_generic);
      exit (EXIT_FAILURE);
    }

  command = argv[1];

  if (strcasecmp (command, "send") == 0)
    return client_send (argc, argv);
  else if (strcasecmp (command, "query") == 0)
    return client_query (argc, argv);
  else if (strcmp (command, "-?") == 0 ||
           strcmp (command, "--help") == 0)
    {
      help_display (argv[0], help_generic);
      exit (EXIT_SUCCESS);
    }
  else if (strcmp (command, "--version") == 0)
    {
      version_display ();
      exit (EXIT_SUCCESS);
    }
  else
    {
      fprintf (stderr, "Unknown command: '%s'\n", command);
      help_display (argv[0], help_generic);
      exit (EXIT_FAILURE);
    }
}
