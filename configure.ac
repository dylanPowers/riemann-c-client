# -*- Autoconf -*-

AC_INIT([riemann-c-client], [0], [algernon@madhouse-project.org])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([no])])

AC_CONFIG_SRCDIR([lib/client.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CC_C_O

LT_INIT([shared])

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(CHECK, check,,)

AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_FUNCS([])
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
