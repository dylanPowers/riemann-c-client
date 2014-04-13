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

#include "cmd-send.c"
#include "cmd-query.c"

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
  else if (strcmp (command, "-V") == 0 ||
           strcmp (command, "--version") == 0)
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
