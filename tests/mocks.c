/* tests/mocks.c -- Riemann C client library
 * Copyright (C) 2014  Gergely Nagy <algernon@madhouse-project.org>
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

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "mocks.h"

#define MOCK(name, ...)                         \
  if (!real_##name)                             \
    real_##name = dlsym (RTLD_NEXT, #name);     \
  if (!mock_##name)                             \
    mock_##name = real_##name;                  \
                                                \
  return mock_##name (__VA_ARGS__)

int
socket (int domain, int type, int protocol)
{
  MOCK (socket, domain, type, protocol);
}

ssize_t
send (int sockfd, const void *buf, size_t len, int flags)
{
  MOCK (send, sockfd, buf, len, flags);
}

ssize_t
sendto (int sockfd, const void *buf, size_t len, int flags,
        const struct sockaddr *dest_addr, socklen_t addrlen)
{
  MOCK (sendto, sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t
recv (int sockfd, void *buf, size_t len, int flags)
{
  MOCK (recv, sockfd, buf, len, flags);
}
