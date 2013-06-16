/* riemann/client.c -- Riemann C client library
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

#include <riemann/client.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "config.h"

const char *
riemann_client_version (void)
{
  return PACKAGE_VERSION;
}

const char *
riemann_client_version_string (void)
{
  return PACKAGE_STRING;
}

struct _riemann_client_t
{
  riemann_client_type_t type;
  int sock;

  struct addrinfo *srv_addr;
};

riemann_client_t *
riemann_client_new (void)
{
  riemann_client_t *client;

  client = malloc (sizeof (riemann_client_t));

  client->type = RIEMANN_CLIENT_NONE;
  client->sock = -1;
  client->srv_addr = NULL;

  return client;
}

int
riemann_client_disconnect (riemann_client_t *client)
{
  if (!client || client->sock == -1)
    return -ENOTCONN;

  if (close (client->sock) != 0)
    return -errno;
  client->sock = -1;

  if (client->srv_addr)
    freeaddrinfo (client->srv_addr);
  client->srv_addr = NULL;

  return 0;
}

void
riemann_client_free (riemann_client_t *client)
{
  if (!client)
    return;

  errno = -riemann_client_disconnect (client);

  free (client);
}

int
riemann_client_connect (riemann_client_t *client,
                        riemann_client_type_t type,
                        const char *hostname, int port)
{
  struct addrinfo hints, *res;
  int sock;

  if (!client || !hostname)
    return -EINVAL;
  if (port <= 0)
    return -ERANGE;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_INET;

  if (type == RIEMANN_CLIENT_TCP)
    hints.ai_socktype = SOCK_STREAM;
  else if (type == RIEMANN_CLIENT_UDP)
    hints.ai_socktype = SOCK_DGRAM;
  else
    return -EINVAL;

  if (getaddrinfo (hostname, NULL, &hints, &res) != 0)
    return -EADDRNOTAVAIL;

  sock = socket (res->ai_family, res->ai_socktype, 0);
  if (sock == -1)
    return -errno;

  ((struct sockaddr_in *)res->ai_addr)->sin_port = htons (port);

  if (connect (sock, res->ai_addr, res->ai_addrlen) != 0)
    return -errno;

  riemann_client_disconnect (client);

  client->type = type;
  client->sock = sock;
  client->srv_addr = res;

  return 0;
}

riemann_client_t *
riemann_client_create (riemann_client_type_t type,
                       const char *hostname, int port)
{
  riemann_client_t *client;
  int e;

  client = riemann_client_new ();
  if (!client)
    return NULL;

  e = riemann_client_connect (client, type, hostname, port);
  if (e != 0)
    {
      riemann_client_free (client);
      errno = -e;
      return NULL;
    }

  return client;
}

static int
_riemann_client_send_message_tcp (riemann_client_t *client,
                                  riemann_message_t *message)
{
  uint8_t *buffer;
  size_t len;
  ssize_t sent;

  buffer = riemann_message_to_buffer (message, &len);
  if (!buffer)
    return -errno;

  sent = send (client->sock, buffer, len, 0);
  if (sent != len)
    {
      int e = errno;

      free (buffer);
      return -e;
    }
  free (buffer);
  return 0;
}

int
riemann_client_send_message (riemann_client_t *client,
                             riemann_message_t *message)
{
  if (!client)
    return -ENOTCONN;
  if (!message)
    return -EINVAL;

  switch (client->type)
    {
    case RIEMANN_CLIENT_TCP:
      return _riemann_client_send_message_tcp (client, message);
    case RIEMANN_CLIENT_UDP:
      return -ENOSYS;
    default:
      return -ENOTCONN;
    }

  return 0;
}
