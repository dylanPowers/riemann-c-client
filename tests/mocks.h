/* tests/mocks.h -- Riemann C client library
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

#ifndef __MADHOUSE_RIEMANN_TESTS_MOCKS_H__
#define __MADHOUSE_RIEMANN_TESTS_MOCKS_H__ 1

#define make_mock(name, rval, ...)                                      \
  static rval (*mock_##name) (__VA_ARGS__);                             \
  static rval (*real_##name) (__VA_ARGS__);                             \
                                                                        \
  static inline void mock_##name##_with (void *x) { mock_##name = x; }  \
  static inline void restore_##name () { mock_##name = real_##name; }

make_mock (socket, int, int, int, int);
make_mock (send, ssize_t, int, const void *, size_t, int);

#endif
