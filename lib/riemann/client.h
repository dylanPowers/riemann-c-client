/* riemann/client.h -- Riemann C client library
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

/** @file lib/riemann/client.h
 */

/** @defgroup riemann_client Riemann Client
 *
 * The network-connected Riemann client to send and receive messages
 * over.
 *
 * @see riemann_message
 *
 * @addtogroup riemann_client
 * @{
 */

#ifndef __MADHOUSE_RIEMANN_CLIENT_H__
#define __MADHOUSE_RIEMANN_CLIENT_H__

#include <riemann/message.h>

/** Supported Riemann client types.
 */
typedef enum
  {
    RIEMANN_CLIENT_NONE, /**< Unspecified. Used for error handling only. */
    RIEMANN_CLIENT_TCP, /**< TCP, for both events and queries. */
    RIEMANN_CLIENT_UDP, /**< UDP, for events only. */
  } riemann_client_type_t;

/** The Riemann Client object.
 *
 * This is an opaque class, the internal state of the Riemann client.
 */
typedef struct _riemann_client_t riemann_client_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Return the library version.
 *
 * @returns the compiled-in library version as a string.
 */
const char *riemann_client_version (void);
/** Return the library name and version.
 *
 * @returns the compiled-in library name and version, as a string.
 */
const char *riemann_client_version_string (void);

/** Allocate a new, unconnected client.
 *
 * Use riemann_client_connect() to connect the object to a Riemann
 * server.
 *
 * @returns a newly allocated client object, which must be freed with
 * riemann_client_free() once no longer needed.
 */
riemann_client_t *riemann_client_new (void);

/** Create a new, connected client.
 *
 * @param type is the client type to use.
 * @param hostname is the hostname to connect to.
 * @param port is the port number to connect to on the server.
 *
 * @retval NULL is returned on error, and `errno` is set to any of the
 * values riemann_client_connect() can set it to.
 * @retval riemann_client_t object is returned on success, which must
 * be freed with riemann_client_free() once no longer needed.
 */
riemann_client_t *riemann_client_create (riemann_client_type_t type,
                                         const char *hostname, int port);

/** Get the file descriptor associated to a client.
 *
 * To allow callers to set socket options and the like on an already
 * connected client, this function can be used to return the file
 * descriptor.
 *
 * @param client is the object whose file descriptor the caller wants.
 *
 * @retval -EINVAL if the client is NULL.
 * @retval -1 if the client is not connected.
 * @retval fd otherwise.
 */
int riemann_client_get_fd (riemann_client_t *client);

/** Free a Riemann client.
 *
 * Disconnects and frees up the supplied client. Sets `errno` to
 * `EINVAL` if the supplied client object is NULL, to `ENOTCONN` if
 * the client is not connected. Can also set `errno` to any value
 * `close()` can set it to.
 *
 * @param client is the client object to disconnect and free up.
 *
 * On success, `errno` is not touched at all.
 */
void riemann_client_free (riemann_client_t *client);

/** Create a new, connected client.
 *
 * @param client is the client to connect to a Riemann server.
 * @param type is the client type to use.
 * @param hostname is the hostname to connect to.
 * @param port is the port number to connect to on the server.
 *
 * @retval NULL is returned on error, and `errno` is set to either
 * `EINVAL`, `ERANGE`, `EADDRNOTAVAIL`, or any of the errno values
 * `socket()` and `connect()` can set.
 * @retval riemann_client_t object is returned on success.
 */
int riemann_client_connect (riemann_client_t *client, riemann_client_type_t type,
                            const char *hostname, int port);

/** Disconnect an existing client.
 *
 * @param client is the client to disconnect.
 *
 * @retval 0 is returned on success.
 * @retval -ENOTCONN is returned if the client is NULL, or not
 * connected.
 * @retval -errno is returned if `close()` fails.
 *
 * @note The client is only disconnected, not freed. It can be
 * re-used, and connected again at a later point.
 */
int riemann_client_disconnect (riemann_client_t *client);

/** Send a message through a client.
 *
 * @param client is the client to send the message through.
 * @param message is the message to send.
 *
 * The message remains untouched, and can be sent with other client
 * instances too.
 *
 * @retval 0 is returned on success.
 * @retval -ENOTCONN is returned if the client is NULL, or not connected.
 * @retval -EINVAL is returned if the message is NULL.
 * @retval -errno is returned on any other error (from `send()`,
 * `sendto()`, etc).
 */
int riemann_client_send_message (riemann_client_t *client,
                                 riemann_message_t *message);

/** Send a message through a client, and free it.
 *
 * @param client is the client to send the message through.
 * @param message is the message to send.
 *
 * A convenience function that calls riemann_message_free() after
 * sending the message with riemann_client_send_message().
 *
 * @returns the same values as riemann_client_send_message().
 */
int riemann_client_send_message_oneshot (riemann_client_t *client,
                                         riemann_message_t *message);

/** Receive a reply message.
 *
 * @param client is the client object to read a reply message from.
 *
 * Only works over TCP, and only after a query was previously sent
 * over the same client.
 *
 * @retval NULL on error, in which case errno is set.
 * @retval riemann_message_t instance on success.
 */
riemann_message_t *riemann_client_recv_message (riemann_client_t *client);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
