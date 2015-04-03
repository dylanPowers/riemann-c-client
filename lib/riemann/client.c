/* riemann/client.c -- Riemann C client library
 * Copyright (C) 2013, 2014, 2015  Gergely Nagy <algernon@madhouse-project.org>
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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>

#include "riemann/_private.h"
#include "riemann/platform.h"

#if HAVE_GNUTLS
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#endif

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

#if HAVE_GNUTLS
static int _riemann_client_send_message_tls (riemann_client_t *client,
                                             riemann_message_t *message);
static riemann_message_t *_riemann_client_recv_message_tls (riemann_client_t *client);
#endif

#if HAVE_GNUTLS
static void
_riemann_client_init_tls (riemann_client_t *client)
{
  client->tls.session = NULL;
  client->tls.creds = NULL;
}
#else
static void
_riemann_client_init_tls (riemann_client_t __attribute__((unused)) *client)
{
}
#endif

riemann_client_t *
riemann_client_new (void)
{
  riemann_client_t *client;

  client = (riemann_client_t *) malloc (sizeof (riemann_client_t));

  client->sock = -1;
  client->srv_addr = NULL;
  client->send = NULL;
  client->recv = NULL;
  _riemann_client_init_tls (client);

  return client;
}

#if HAVE_GNUTLS
static void
_riemann_client_disconnect_tls (riemann_client_t *client)
{
  if (client->send == _riemann_client_send_message_tls)
    {
      if (client->tls.session)
        {
          gnutls_deinit (client->tls.session);
          client->tls.session = NULL;
        }

      if (client->tls.creds)
        {
          gnutls_certificate_free_credentials (client->tls.creds);
          client->tls.creds = NULL;
        }
    }
}
#else
static void
_riemann_client_disconnect_tls (riemann_client_t __attribute__((unused)) *client)
{
}
#endif

int
riemann_client_disconnect (riemann_client_t *client)
{
  if (!client || client->sock == -1)
    return -ENOTCONN;

  _riemann_client_disconnect_tls (client);

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

#if HAVE_GNUTLS
static int
_verify_certificate_callback (gnutls_session_t session)
{
  unsigned int status;
  int ret;
  const char *hostname;
  gnutls_typed_vdata_st data[2];

  hostname = (const char *) gnutls_session_get_ptr (session);

  memset (data, 0, sizeof (data));

  data[0].type = GNUTLS_DT_DNS_HOSTNAME;
  data[0].data = (unsigned char *) hostname;

  data[1].type = GNUTLS_DT_KEY_PURPOSE_OID;
  data[1].data = (unsigned char *) GNUTLS_KP_TLS_WWW_SERVER;

  ret = gnutls_certificate_verify_peers (session, data, 2,
                                         &status);
  if (ret < 0 || status != 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

   return 0;
}
#endif

static void
_riemann_client_connect_setup_tcp (riemann_client_t *client,
                                   struct addrinfo *hints)
{
  client->send = _riemann_client_send_message_tcp;
  client->recv = _riemann_client_recv_message_tcp;

  hints->ai_socktype = SOCK_STREAM;
}

static void
_riemann_client_connect_setup_udp (riemann_client_t *client,
                                   struct addrinfo *hints)
{
  client->send = _riemann_client_send_message_udp;
  client->recv = _riemann_client_recv_message_udp;

  hints->ai_socktype = SOCK_DGRAM;
}

typedef struct
{
  char *cafn;
  char *certfn;
  char *keyfn;
  unsigned int handshake_timeout;
} riemann_client_tls_options_t;

#if HAVE_GNUTLS
static int
_riemann_client_connect_setup_tls (riemann_client_t *client,
                                   struct addrinfo *hints,
                                   va_list aq,
                                   riemann_client_tls_options_t *tls_options)
{
  va_list ap;
  riemann_client_option_t option;

  memset (tls_options, 0, sizeof (riemann_client_tls_options_t));
  tls_options->handshake_timeout = GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT;

  client->send = _riemann_client_send_message_tls;
  client->recv = _riemann_client_recv_message_tls;

  hints->ai_socktype = SOCK_STREAM;

  va_copy (ap, aq);

  option = (riemann_client_option_t) va_arg (ap, int);
  do
    {
      switch (option)
        {
        case RIEMANN_CLIENT_OPTION_NONE:
          break;

        case RIEMANN_CLIENT_OPTION_TLS_CA_FILE:
          tls_options->cafn = va_arg (ap, char *);
          break;

        case RIEMANN_CLIENT_OPTION_TLS_CERT_FILE:
          tls_options->certfn = va_arg (ap, char *);
          break;

        case RIEMANN_CLIENT_OPTION_TLS_KEY_FILE:
          tls_options->keyfn = va_arg (ap, char *);
          break;

        case RIEMANN_CLIENT_OPTION_TLS_HANDSHAKE_TIMEOUT:
          tls_options->handshake_timeout = va_arg (ap, unsigned int);
          break;

        default:
          va_end (ap);
          return -EINVAL;
        }

      if (option != RIEMANN_CLIENT_OPTION_NONE)
        option = (riemann_client_option_t) va_arg (ap, int);
    }
  while (option != RIEMANN_CLIENT_OPTION_NONE);
  va_end (ap);

  if (!tls_options->cafn || !tls_options->certfn || !tls_options->keyfn)
    {
      return -EINVAL;
    }

  return 0;
}
#else
static void
_riemann_client_connect_setup_tls (riemann_client_t __attribute__((unused)) *client,
                                   struct addrinfo __attribute__((unused)) *hints,
                                   va_list __attribute__((unused)) aq,
                                   riemann_client_tls_options_t __attribute__((unused)) *tls_options)
{
  return -ENOSYS;
}
#endif

#if HAVE_GNUTLS
static int
_riemann_client_connect_tls_handshake (riemann_client_t *client,
                                       riemann_client_tls_options_t *tls_options)
{
  int e;

  gnutls_certificate_allocate_credentials (&(client->tls.creds));
  gnutls_certificate_set_x509_trust_file (client->tls.creds, tls_options->cafn,
                                          GNUTLS_X509_FMT_PEM);
  gnutls_certificate_set_verify_function (client->tls.creds,
                                          _verify_certificate_callback);

  gnutls_certificate_set_x509_key_file (client->tls.creds,
                                        tls_options->certfn, tls_options->keyfn,
                                        GNUTLS_X509_FMT_PEM);

  gnutls_init (&client->tls.session, GNUTLS_CLIENT);

  gnutls_set_default_priority (client->tls.session);

  gnutls_credentials_set (client->tls.session, GNUTLS_CRD_CERTIFICATE,
                          client->tls.creds);

  gnutls_transport_set_int (client->tls.session, client->sock);
  gnutls_handshake_set_timeout (client->tls.session,
                                tls_options->handshake_timeout);

  do {
    e = gnutls_handshake (client->tls.session);
  }
  while (e < 0 && gnutls_error_is_fatal (e) == 0);

  if (e != 0)
    {
      gnutls_deinit (client->tls.session);
      gnutls_certificate_free_credentials (client->tls.creds);

      client->tls.session = NULL;
      client->tls.creds = NULL;

      return -EPROTO;
    }

  return 0;
}
#else
static int
_riemann_client_connect_tls_handshake (riemann_client_t __attribute__((unused)) *client,
                                       riemann_client_tls_options_t __attribute__((unused)) *tls_options)
{
  return -ENOSYS;
}
#endif

static int
riemann_client_connect_va (riemann_client_t *client,
                           riemann_client_type_t type,
                           const char *hostname, int port,
                           va_list aq)
{
  struct addrinfo hints, *res;
  int sock;
  riemann_client_tls_options_t tls_options;

  if (!client || !hostname)
    return -EINVAL;
  if (port <= 0)
    return -ERANGE;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;

  switch (type)
    {
    case RIEMANN_CLIENT_TCP:
      _riemann_client_connect_setup_tcp (client, &hints);
      break;
    case RIEMANN_CLIENT_UDP:
      _riemann_client_connect_setup_udp (client, &hints);
      break;
    case RIEMANN_CLIENT_TLS:
      {
        va_list ap;
        int e;

        va_copy (ap, aq);
        e = _riemann_client_connect_setup_tls (client, &hints, ap, &tls_options);
        va_end (ap);

        if (e != 0)
          return e;

        break;
      }
    default:
      return -EINVAL;
    }

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

  if (type == RIEMANN_CLIENT_TLS)
    return _riemann_client_connect_tls_handshake (client, &tls_options);

  return 0;
}

int
riemann_client_connect (riemann_client_t *client,
                        riemann_client_type_t type,
                        const char *hostname, int port, ...)
{
  va_list ap;
  int r;

  va_start (ap, port);
  r = riemann_client_connect_va (client, type, hostname, port, ap);
  va_end (ap);
  return r;
}

int
riemann_client_connect_1_0 (riemann_client_t *client,
                            riemann_client_type_t type,
                            const char *hostname, int port)
{
  return riemann_client_connect (client, type, hostname, port);
}

#ifdef HAVE_VERSIONING
__asm__(".symver riemann_client_connect_1_0,riemann_client_connect@RIEMANN_C_1.0");
#endif

riemann_client_t *
riemann_client_create (riemann_client_type_t type,
                       const char *hostname, int port, ...)
{
  riemann_client_t *client;
  int e;
  va_list ap;

  client = riemann_client_new ();

  va_start (ap, port);
  e = riemann_client_connect_va (client, type, hostname, port, ap);
  if (e != 0)
    {
      riemann_client_free (client);
      va_end (ap);
      errno = -e;
      return NULL;
    }
  va_end (ap);

  return client;
}

riemann_client_t *
riemann_client_create_1_0 (riemann_client_type_t type,
                           const char *hostname, int port)
{
  return riemann_client_create (type, hostname, port);
}

#ifdef HAVE_VERSIONING
__asm__(".symver riemann_client_create_1_0,riemann_client_create@RIEMANN_C_1.0");
#endif

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

#if HAVE_GNUTLS
static int
_riemann_client_send_message_tls (riemann_client_t *client,
                                  riemann_message_t *message)
{
  uint8_t *buffer;
  size_t len;
  ssize_t sent;

  buffer = riemann_message_to_buffer (message, &len);
  if (!buffer)
    return -errno;

  sent = gnutls_record_send (client->tls.session, buffer, len);
  if (sent < 0 || (size_t)sent != len)
    {
      free (buffer);
      return -EPROTO;
    }
  free (buffer);
  return 0;
}
#endif

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

  buffer = (uint8_t *) malloc (len);

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

#if HAVE_GNUTLS
static riemann_message_t *
_riemann_client_recv_message_tls (riemann_client_t *client)
{
  uint32_t header, len;
  uint8_t *buffer;
  ssize_t received;
  riemann_message_t *message;

  received = gnutls_record_recv (client->tls.session, &header, sizeof (header));
  if (received != sizeof (header))
    {
      errno = EPROTO;
      return NULL;
    }
  len = ntohl (header);

  buffer = (uint8_t *) malloc (len);

  received = gnutls_record_recv (client->tls.session, buffer, len);
  if (received != len)
    {
      free (buffer);
      errno = EPROTO;
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
#endif

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
