/* riemann/simple.h -- Riemann C client library
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

/** @file lib/riemann/simple.h
 */

/** @defgroup riemann_simple Simple helpers for one-off messages
 *
 * These are various helper functions to send one-off messages to a
 * Riemann server.
 *
 * @addtogroup riemann_simple
 * @{
 */

#ifndef __MADHOUSE_RIEMANN_SIMPLE_H__
#define __MADHOUSE_RIEMANN_SIMPLE_H__ 1

#include <riemann/riemann-client.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Send a one-off event-filled message.
 *
 * @param client is the client to send with.
 * @param field is the first field.
 * @param ... are the field arguments, and further field descriptions,
 * terminated by #RIEMANN_EVENT_FIELD_NONE.
 *
 * Constructs a new message like if
 * riemann_message_create_with_events() was called with the same
 * arguments, sends that through the supplied client, and then frees
 * up the message.
 *
 * Intended for one-off messages.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_send (riemann_client_t *client,
                  riemann_event_field_t field, ...);

/** Send a one-off event-filled message.
 *
 * @param client is the client to send with.
 * @param field is the first field.
 * @param aq the field arguments, and further field descriptions,
 * terminated by #RIEMANN_EVENT_FIELD_NONE, wrapped in a `va_list`.
 *
 * Constructs a new message like if
 * riemann_message_create_with_events_va() was called with the same
 * arguments, sends that through the supplied client, and then frees
 * up the message.
 *
 * Intended for one-off messages.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_send_va (riemann_client_t *client,
                     riemann_event_field_t field, va_list aq);

/** Send a one-off query.
 *
 * @param client is the client to query with.
 * @param query is the query string to use.
 *
 * Constructs a new query object, as if riemann_query_new() was called
 * with the same argument, sends the query, and waits for a reply. The
 * reply is parsed and returned, and the query freed.
 *
 * @retval riemann_message_t object on success, the result of the
 * query. The object must be freed with riemann_message_free() once no
 * longer needed.
 * @retval NULL is returned upon failure, and `errno` is set
 * appropriately in this case.
 */
riemann_message_t *riemann_query (riemann_client_t *client,
                                  const char *query);

/** Send & receive a message.
 *
 * Sends a message, and if need be, waits for a reply.
 *
 * @param client is the client to send and receive with.
 * @param message is the message to send.
 *
 * @note The #message is freed before the function returns.
 *
 * @returns The newly allocated response message (query results, ACK
 * over TCP and TLS, generated response on UDP), or NULL on
 * communication error.
 *
 */
riemann_message_t *riemann_communicate (riemann_client_t *client,
                                        riemann_message_t *message);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
