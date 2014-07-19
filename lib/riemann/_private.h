/* riemann/_private.h -- Riemann C client library
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

#ifndef __MADHOUSE_RIEMANN_PRIVATE_H__
#define __MADHOUSE_RIEMANN_PRIVATE_H__ 1

#include <riemann/riemann-client.h>
#include <stdarg.h>

int riemann_event_set_va (riemann_event_t *event,
                          riemann_event_field_t first_field, va_list aq);

typedef int (*riemann_client_send_message_t) (riemann_client_t *client,
                                              riemann_message_t *message);
typedef riemann_message_t *(*riemann_client_recv_message_t) (riemann_client_t *client);

struct _riemann_client_t
{
  int sock;
  struct addrinfo *srv_addr;

  riemann_client_send_message_t send;
  riemann_client_recv_message_t recv;
};

#endif
