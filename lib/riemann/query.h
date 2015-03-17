/* riemann/query.h -- Riemann C client library
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

/** @file lib/riemann/query.h
 */

/** @defgroup riemann_queries Riemann Queries
 *
 * Queries can be used to do queries against the Riemann Index.
 *
 * @addtogroup riemann_queries
 * @{
 */

#ifndef __MADHOUSE_RIEMANN_QUERY_H__
#define __MADHOUSE_RIEMANN_QUERY_H__ 1

#include <riemann/proto/riemann.pb-c.h>

/** A Riemann query object.
 *
 * The only field is `string`.
 */
typedef Query riemann_query_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Create a new query object.
 *
 * @param string is the query string. A copy is made of it, ownership
 * remains at the caller.
 *
 * @returns A newly allocated #riemann_query_t object, which must be
 * freed with riemann_query_free() once no longer needed.
 */
riemann_query_t *riemann_query_new (const char *string);

/** Clone a query object.
 *
 * @param query is the object to make a copy of.
 *
 * A full copy is made of the query, the cloned object shares no data
 * with the old one. The original query is left untouched.
 *
 * @retval riemann_query_t object is returned on success, and must be
 * freed with riemann_query_free() once no longer needed.
 * @retval NULL is returned on failure, and `errno` is set in this
 * case.
 */
riemann_query_t *riemann_query_clone (const riemann_query_t *query);

/** Free a query object.
 *
 * @param query is the query object to free.
 *
 * @note Sets `errno` if the query was NULL.
 */
void riemann_query_free (riemann_query_t *query);

/** Set the query string of a query object.
 *
 * @param query is the object to set the query string of.
 * @param string is the query string to set. A copy is made of it,
 * ownership remains at the caller.
 *
 * @retval 0 is returned on success.
 * @retval -errno is returned on failure.
 */
int riemann_query_set_string (riemann_query_t *query, const char *string);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif
