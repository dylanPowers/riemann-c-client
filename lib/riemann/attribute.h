/* riemann/attribute.h -- Riemann C client library
 * Copyright (C) 2013, 2015  Gergely Nagy <algernon@madhouse-project.org>
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

/** @file lib/riemann/attribute.h
 */

#ifndef __MADHOUSE_RIEMANN_ATTRIBUTE_H__
#define __MADHOUSE_RIEMANN_ATTRIBUTE_H__ 1

#include <riemann/proto/riemann.pb-c.h>

/** @defgroup riemann_attribute Riemann Attributes
 *
 * Attributes can be attached to events, they are user-specified
 * key-value pairs.
 *
 * @see riemann_event
 *
 * @addtogroup riemann_attribute
 * @{
 */

/** An attribute.
 *
 * Attributes are key-value pairs, they have `key` and `value`
 * members.
 */
typedef Attribute riemann_attribute_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Allocate a new attribute.
 *
 * Allocates a new - empty - attribute, with neither key nor value
 * set. The returned object must be freed with
 * riemann_attribute_free().
 *
 * @returns A newly allocated attribute object.
 */
riemann_attribute_t *riemann_attribute_new (void);

/** Clone an existing attribute.
 *
 * @param attrib is the existing attribute to copy.
 *
 * The returned copy must be freed with riemann_attribute_free(). The
 * original will remain untouched.
 *
 * @retval NULL is returned on error, and `errno` will be set to
 * `EINVAL` if the supplied attribute was NULL.
 * @retval riemann_attribute_t object - a copy of the supplied
 * attribute - is returned otherwise.
 */
riemann_attribute_t *riemann_attribute_clone (const riemann_attribute_t *attrib);

/** Create a new attribute with a key-value pair.
 *
 * @param key is the optional key part of the attribute.
 * @param value is the optional value part of the attribute.
 *
 * With both arguments set to NULL, this is equivalent to
 * riemann_attribute_new(). Similarly, the returned object must be
 * freed with riemann_attribute_free().
 *
 * Both `key` and `value` will be copied, ownership remains at the
 * caller.
 *
 * @returns A newly allocated object with key and value already set.
 */
riemann_attribute_t *riemann_attribute_create (const char *key,
                                               const char *value);
/** Free an attribute.
 *
 * @param attrib is the attribute to free.
 *
 * @note Sets `errno` to `EINVAL` in case the supplied attribute was
 * NULL.
 */
void riemann_attribute_free (riemann_attribute_t *attrib);

/** Set the key part of an attribute.
 *
 * @param attrib is the attribute to set the key part of.
 * @param key is the key to use.
 *
 * The key will be copied internally, ownership remains at the caller.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_attribute_set_key (riemann_attribute_t *attrib, const char *key);

/** Set the value part of an attribute.
 *
 * @param attrib is the attribute to set the key part of.
 * @param value is the value to use.
 *
 * The value will be copied internally, ownership remains at the caller.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_attribute_set_value (riemann_attribute_t *attrib, const char *value);

/** Set the both key and value parts of an attribute.
 *
 * @param attrib is the attribute to set the key and value of.
 * @param key is the key to use.
 * @param value is the value to use.
 *
 * Both `key` and `value` will be copied, ownership remains at the
 * caller. If either of the arguments is NULL, the function will
 * fail. However, setting both properties is not atomic: if the
 * function fails, one of the properties may have been changed
 * already. In this case, it is best to discard the object altogether.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_attribute_set (riemann_attribute_t *attrib,
                           const char *key, const char *value);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
