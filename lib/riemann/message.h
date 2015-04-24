/* riemann/message.h -- Riemann C client library
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

/** @file lib/riemann/message.h
 */

/** @defgroup riemann_message Riemann Messages
 *
 * Messages are the high-level elements that are serialized on the
 * wire when communicating with Riemann.
 *
 * @see riemann_event
 * @see riemann_queries
 * @see riemann_client
 *
 * @addtogroup riemann_message
 * @{
 */

#ifndef __MADHOUSE_RIEMANN_MESSAGE_H__
#define __MADHOUSE_RIEMANN_MESSAGE_H__ 1

#include <riemann/proto/riemann.pb-c.h>
#include <riemann/event.h>
#include <riemann/query.h>

/** Riemann Message type.
 *
 * Fields are `query`, `events` (and `n_events`) for event and query
 * messages. The `ok` (and `has_ok`) and `error` fields are for
 * received messages (on top of the others).
 */
typedef Msg riemann_message_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Allocate a new, empty message.
 *
 * @returns A newly allocated riemann_message_t object, which must be
 * freed with riemann_message_free() once no longer needed.
 */
riemann_message_t *riemann_message_new (void);

/** Create a new message, with events.
 *
 * Takes a NULL-terminated list of events, and takes ownership of
 * them.
 *
 * @returns A newly allocated riemann_message_t object, with the
 * supplied events already set. The object must be freed with
 * riemann_message_free() once no longer needed.
 */
riemann_message_t *riemann_message_create_with_events (riemann_event_t *event, ...);

/** Create a new message, with events.
 *
 * Instead of taking a NULL-terminates list of events like
 * riemann_message_create_with_events(), takes an event and a
 * `va_list`, otherwise behaves the same.
 *
 * @returns A newly allocated riemann_message_t object, with the
 * supplied events already set. The object must be freed with
 * riemann_message_free() once no longer needed.
 */
riemann_message_t *riemann_message_create_with_events_va (riemann_event_t *event, va_list aq);

/** Create a new message, with a query.
 *
 * @param query is the query to embed. Ownership is transferred to the
 * message.
 *
 * @returns A newly created riemann_message_t object, with the query
 * embedded. The object must be freed with riemann_message_free() once
 * no longer needed.
 */
riemann_message_t *riemann_message_create_with_query (riemann_query_t *query);

/** Deep-copy a message.
 *
 * @param message is the message to copy.
 *
 * The message and all its properties will be cloned into a new
 * message. No data will be shared between the old and new
 * messages. The original message is left untouched.
 *
 * @returns A new clone of the message, which must be freed with
 * riemann_message_free(0 once no longer needed.
 */
riemann_message_t *riemann_message_clone (const riemann_message_t *message);

/** Free a riemann message.
 *
 * @param message is the message to free up.
 *
 * Frees up all resources - events, the query, attributes, etc -
 * allocated to the message, and the message itself.
 *
 * @note Sets `errno` on failure.
 */
void riemann_message_free (riemann_message_t *message);

/** Set the events of a message.
 *
 * @param message is the message one wants to set the events of.
 * @param n_events is the number of events to set.
 * @param events is an array of at least n_events of #riemann_event_t
 * objects.
 *
 * @note The @a events arrays ownership is transferred to the message,
 * the caller should not change neither the array, nor the events
 * contained within it afterwards.
 *
 * @note If the message previously had any events embedded, they will
 * be freed first.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_set_events_n (riemann_message_t *message,
                                  size_t n_events,
                                  riemann_event_t **events);

/** Set the events of a message.
 *
 * @param message is the message one wants to set the events of.
 * @param ... is a NULL-terminated list of #riemann_event_t objects.
 *
 * @note Ownership of the supplied events are transferred to the
 * message, the caller should not change neither the array, nor the
 * events contained within it afterwards.
 *
 * @note If the message previously had any events embedded, they will
 * be freed first.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_set_events (riemann_message_t *message, ...);

/** Set the events of a message.
 *
 * @param message is the  message one wants to set the events of.
 * @param aq are the events wrapped in `va_list` to use.
 *
 * @note Ownership of the supplied events are transferred to the
 * message, the caller should not change neither the array, nor the
 * events contained within it afterwards.
 *
 * @note If the message previously had any events embedded, they will
 * be freed first.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_set_events_va (riemann_message_t *message, va_list aq);

/** Append events to a message.
 *
 * @param message is the message to append events to.
 * @param n_events is the number of events to append.
 * @param events is an array of at least n_events of #riemann_event_t
 * objects.
 *
 * @note The @a events arrays ownership is transferred to the message,
 * the caller should not change neither the array, nor the events
 * contained within it afterwards.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_append_events_n (riemann_message_t *message,
                                     size_t n_events,
                                     riemann_event_t **events);

/** Append events to a message.
 *
 * @param message is the message to append events to.
 * @param ... is a NULL-terminated list of #riemann_event_t objects.
 *
 * @note Ownership of the supplied events are transferred to the
 * message, the caller should not change neither the array, nor the
 * events contained within it afterwards.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_append_events (riemann_message_t *message, ...);

/** Append events to a message.
 *
 * @param message is the message to append events to.
 * @param aq are the events wrapped in `va_list` to use.
 *
 * @note Ownership of the supplied events are transferred to the
 * message, the caller should not change neither the array, nor the
 * events contained within it afterwards.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_append_events_va (riemann_message_t *message, va_list aq);

/** Set the query of a message.
 *
 * @param message is the message to set the query of.
 * @param query is the query to set.
 *
 * @note Ownership of the query is transferred to the message, the
 * caller should not change it in any way afterwards.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_message_set_query (riemann_message_t *message,
                               riemann_query_t *query);

/** Serialize a message to a byte array.
 *
 * @param message is the message to serialize.
 * @param len is the output parameter that will hold the length of the
 * serialized array.
 *
 * @retval "uint8_t *" - the message serialized to a byte array. With
 * @a len set to its length.
 * @retval NULL on failure, in which case `errno` is set appropriately.
 */
uint8_t *riemann_message_to_buffer (riemann_message_t *message, size_t *len);

/** Parse a byte array into a message object.
 *
 * @param buffer is the byte array to parse.
 * @param len is the length of the byte array.
 *
 * @retval riemann_message_t object is returned on success.
 * @retval NULL is returned on failure, and `errno` is set
 * appropriately.
 */
riemann_message_t *riemann_message_from_buffer (uint8_t *buffer, size_t len);

/** Get the packed size of a message object.
 *
 * @param message is the message to get the packed size of.
 *
 * @returns the packed message size, or 0 on error (in which case,
 * errno is set appropriately).
 */
size_t riemann_message_get_packed_size (riemann_message_t *message);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
