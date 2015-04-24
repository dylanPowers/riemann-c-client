/* riemann/event.h -- Riemann C client library
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

/** @file lib/riemann/event.h
 */

/** @defgroup riemann_event Riemann Events
 *
 * The main building block of interaction with Riemann: Events.
 *
 * @see riemann_message
 *
 * @addtogroup riemann_event
 * @{
 */

#ifndef __MADHOUSE_RIEMANN_EVENT_H__
#define __MADHOUSE_RIEMANN_EVENT_H__ 1

#include <riemann/proto/riemann.pb-c.h>
#include <riemann/attribute.h>
#include <stdarg.h>

/** The Riemann Event protobuf structure.
 *
 * Fields are `time`, `state`, `service`, `host`, `description`,
 * `ttl`, `metric_sint64`, `metric_f`, `metric_d`, `tags` (and
 * `n_tags`), `attributes` (and `n_attributes`).
 *
 * These fields are exactly the same as in the Riemann protocol
 * buffers definition.
 */
typedef Event riemann_event_t;

/** Event field types.
 *
 * These types are used by riemann_event_set() and various other
 * functions, to set various properties of an event in a convenient
 * way.
 */
typedef enum
  {
    /** No field, a signal to end processing. */
    RIEMANN_EVENT_FIELD_NONE = 0,
    /** An alias of RIEMANN_EVENT_FIELD_NONE. */
    RIEMANN_EVENT_EMPTY = 0,
    /** The time (`int64_t`) field. */
    RIEMANN_EVENT_FIELD_TIME,
    /** The state (`char *`) field. */
    RIEMANN_EVENT_FIELD_STATE,
    /** The service (`char *`) field. The argument is copied,
        ownership remains at the caller. */
    RIEMANN_EVENT_FIELD_SERVICE,
    /** The host (`char *`) field. The argument is copied, ownership
        remains at the caller. */
    RIEMANN_EVENT_FIELD_HOST,
    /** The description (`char *`) field. The argument is copied,
        ownership remains at the caller. */
    RIEMANN_EVENT_FIELD_DESCRIPTION,
    /** The tags (a NULL-terminated list of `char *`s). The arguments
        are copied, ownership remains at the caller. */
    RIEMANN_EVENT_FIELD_TAGS,
    /** The TTL (`float`) field. */
    RIEMANN_EVENT_FIELD_TTL,
    /** The attributes (a NULL-terminated list of riemann_attribute_t
        objects). The arguments are borrowed, ownership transfers to
        the event. */
    RIEMANN_EVENT_FIELD_ATTRIBUTES,
    /** The metric field, as a signed 64-bit int. */
    RIEMANN_EVENT_FIELD_METRIC_S64,
    /** The metric field as double. */
    RIEMANN_EVENT_FIELD_METRIC_D,
    /** The metric field as float. */
    RIEMANN_EVENT_FIELD_METRIC_F,
    /** The attributes, as a NULL-terminated list of key-value
        pairs. The arguments are copied, ownership remains at the
        caller. */
    RIEMANN_EVENT_FIELD_STRING_ATTRIBUTES
  } riemann_event_field_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Create a new, empty event.
 *
 * @returns a newly allocated event object, which must be freed with
 * riemann_event_free() once no longer needed.
 */
riemann_event_t *riemann_event_new (void);

/** Creates a new event, with pre-set fields.
 *
 * Takes the same arguments as riemann_event_set().
 *
 * @retval riemann_event_t object on success, which must be freed with
 * riemann_event_free() once no longer needed.
 * @retval NULL on failure, and sets `errno` appropriately.
 */
riemann_event_t *riemann_event_create (riemann_event_field_t field, ...);

/** Creates a new event, with pre-set fields.
 *
 * Similar to riemann_event_create(), but instead of a field &
 * argument list, accepts a `va_list` instead.
 *
 * @returns The same values as riemann_event_create().
 */
riemann_event_t *riemann_event_create_va (riemann_event_field_t field,
                                          va_list aq);

/** Creates a deep copy of the event.
 *
 * @param event is the event to copy.
 *
 * Performs a deep copy: all properties, tags and attributes will be
 * copied, nothing will be shared between the old and new events.
 *
 * @retval riemann_event_t object on success, which must be freed with
 * riemann_event_free() once no longer needed.
 * @retval NULL on failure, in which case `errno` is set appropriately.
 */
riemann_event_t *riemann_event_clone (const riemann_event_t *event);

/** Free up an event.
 *
 * @param event is the event to free up.
 *
 * @note Sets `errno` to `EINVAL` if event is NULL.
 */
void riemann_event_free (riemann_event_t *event);

/** Set any number of fields of an event to new values.
 *
 * @param event is the event to set the fields of.
 * @param ... the rest of the arguments are a list of
 * #riemann_event_field_t fields followed by their arguments (see the
 * #riemann_event_field_t docs!). The list must be terminated by
 * #RIEMANN_EVENT_FIELD_NONE.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_event_set (riemann_event_t *event, ...);

/** Set any number of fields on an event to new values.
 *
 * Same as riemann_event_set(), but most of the arguments are passed
 * via a `va_list` instead of as variadic arguments.
 *
 * @returns the same values as riemann_event_set().
 */
int riemann_event_set_va (riemann_event_t *event,
                          riemann_event_field_t first_field, va_list aq);

/** Convenience macro to set a single field only.
 *
 * @param event is the event to set a single field on.
 * @param field is the field to set (without the
 * `RIEMANN_EVENT_FIELD_` prefix!)
 * @param ... are the arguments for setting the value.
 *
 * @returns the same values as riemann_event_set().
 */
#define riemann_event_set_one(event, field, ...)                        \
  riemann_event_set (event, RIEMANN_EVENT_FIELD_##field, __VA_ARGS__,   \
                     RIEMANN_EVENT_FIELD_NONE)

/** Attach a tag to an event.
 *
 * @param event is the event to attach a tag to.
 * @param tag is the tag to attach. The value is copied, ownership
 * remains at the caller.
 *
 * @note Does not attempt to verify that the tag is unique!
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_event_tag_add (riemann_event_t *event, const char *tag);

/** Attach an attribute to an event.
 *
 * @param event is the event to attach an attribute to.
 * @param attrib is the attribute to attach. The value is borrowed,
 * ownership transfers to the event.
 *
 * @note Does not attempt to verify that the attribute is unique!
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_event_attribute_add (riemann_event_t *event,
                                 riemann_attribute_t *attrib);

/** Attach an attribute to an event.
 *
 * @param event is the event to attach an attribute to.
 * @param key is the key part of the attribute to attach, the value is
 * copied, ownership remains at the caller.
 * @param value is the value part of the attribute to attach, the
 * value is copied, ownership remains at the caller.
 *
 * @note Does not attempt to verify that the key is unique.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_event_string_attribute_add (riemann_event_t *event,
                                        const char *key,
                                        const char *value);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
