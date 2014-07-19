/* riemann/client.c -- Riemann C client library
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

#include <riemann/client.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "riemann/_private.h"
#include "riemann/platform.h"

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

static int _riemann_client_send_message_tcp (riemann_client_t *client,
                                             riemann_message_t *message);
static int _riemann_client_send_message_udp (riemann_client_t *client,
                                             riemann_message_t *message);
static riemann_message_t *_riemann_client_recv_message_tcp (riemann_client_t *client);
static riemann_message_t *_riemann_client_recv_message_udp (riemann_client_t *client);

riemann_client_t *
riemann_client_new (void)
{
  riemann_client_t *client;

  client = malloc (sizeof (riemann_client_t));

  client->sock = -1;
  client->srv_addr = NULL;
  client->send = NULL;
  client->recv = NULL;

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
    {
      errno = EINVAL;
      return;
    }

  errno = -riemann_client_disconnect (client);

  free (client);
}

int
riemann_client_get_fd (riemann_client_t *client)
{
  if (!client)
    return -EINVAL;

  return client->sock;
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
  hints.ai_family = AF_UNSPEC;

  if (type == RIEMANN_CLIENT_TCP)
    {
      client->send = _riemann_client_send_message_tcp;
      client->recv = _riemann_client_recv_message_tcp;

      hints.ai_socktype = SOCK_STREAM;
    }
  else if (type == RIEMANN_CLIENT_UDP)
    {
      client->send = _riemann_client_send_message_udp;
      client->recv = _riemann_client_recv_message_udp;

      hints.ai_socktype = SOCK_DGRAM;
    }
  else
    return -EINVAL;

  if (getaddrinfo (hostname, NULL, &hints, &res) != 0)
    return -EADDRNOTAVAIL;

  sock = socket (res->ai_family, res->ai_socktype, 0);
  if (sock == -1)
    {
      int e = errno;

      freeaddrinfo (res);
      return -e;
    }

  ((struct sockaddr_in *)res->ai_addr)->sin_port = htons (port);

  if (connect (sock, res->ai_addr, res->ai_addrlen) != 0)
    {
      int e = errno;

      freeaddrinfo (res);
      close (sock);

      return -e;
    }

  riemann_client_disconnect (client);

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
  if (sent == -1 || (size_t)sent != len)
    {
      int e = errno;

      free (buffer);
      return -e;
    }
  free (buffer);
  return 0;
}

struct _riemann_buff_w_hdr
{
  uint32_t header;
  uint8_t data[0];
};

static int
_riemann_client_send_message_udp (riemann_client_t *client,
                                  riemann_message_t *message)
{
  struct _riemann_buff_w_hdr *buffer;
  size_t len;
  ssize_t sent;

  buffer = (struct _riemann_buff_w_hdr *)
    riemann_message_to_buffer (message, &len);
  if (!buffer)
    return -errno;

  sent = sendto (client->sock, buffer->data, len - sizeof (buffer->header), 0,
                 client->srv_addr->ai_addr, client->srv_addr->ai_addrlen);
  if (sent == -1 || (size_t)sent != len - sizeof (buffer->header))
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

  if (!client->send)
    return -ENOTCONN;

  return client->send (client, message);
}

int
riemann_client_send_message_oneshot (riemann_client_t *client,
                                     riemann_message_t *message)
{
  int ret = 0;

  ret = riemann_client_send_message (client, message);
  riemann_message_free (message);

  return ret;
}

static riemann_message_t *
_riemann_client_recv_message_tcp (riemann_client_t *client)
{
  uint32_t header, len;
  uint8_t *buffer;
  ssize_t received;
  riemann_message_t *message;

  received = recv (client->sock, &header, sizeof (header), MSG_WAITALL);
  if (received != sizeof (header))
    return NULL;
  len = ntohl (header);

  buffer = malloc (len);

  received = recv (client->sock, buffer, len, MSG_WAITALL);
  if (received != len)
    {
      int e = errno;

      free (buffer);
      errno = e;
      return NULL;
    }

  message = riemann_message_from_buffer (buffer, len);
  if (message == NULL)
    {
      int e = errno;

      free (buffer);
      errno = e;
      return NULL;
    }
  free (buffer);

  return message;
}

static riemann_message_t *
_riemann_client_recv_message_udp (riemann_client_t __attribute__((unused)) *client)
{
  errno = ENOTSUP;
  return NULL;
}

riemann_message_t *
riemann_client_recv_message (riemann_client_t *client)
{
  if (!client || !client->recv)
    {
      errno = ENOTCONN;
      return NULL;
    }

  return client->recv (client);
}
