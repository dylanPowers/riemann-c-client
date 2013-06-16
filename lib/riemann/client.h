/* riemann/client.h -- Riemann C client library
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

#ifndef __MADHOUSE_RIEMANN_CLIENT_H__
#define __MADHOUSE_RIEMANN_CLIENT_H__

typedef enum
  {
    RIEMANN_CLIENT_NONE,
    RIEMANN_CLIENT_TCP,
    RIEMANN_CLIENT_UDP,
  } riemann_client_type_t;

typedef struct _riemann_client_t riemann_client_t;

const char *riemann_client_version (void);
const char * riemann_client_version_string (void);

riemann_client_t *riemann_client_new (void);
riemann_client_t *riemann_client_create (riemann_client_type_t type,
                                         const char *hostname, int port);
void riemann_client_free (riemann_client_t *client);

int riemann_client_connect (riemann_client_t *client, riemann_client_type_t type,
                            const char *hostname, int port);
int riemann_client_disconnect (riemann_client_t *client);

#endif
