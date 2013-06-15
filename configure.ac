# -*- Autoconf -*-

AC_INIT([riemann-c-client], [0], [algernon@madhouse-project.org])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([no])])

AC_CONFIG_SRCDIR([lib/client.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_LIBTOOL

LT_INIT([shared])

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(CHECK, check,,)

AC_CHECK_HEADERS([arpa/inet.h stdlib.h])
AC_CHECK_FUNCS([strdup strerror])
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_INT64_T

# Check for linker version script support.
ac_save_LDFLAGS=$LDFLAGS
AC_CACHE_CHECK(whether $LD supports symbol version scripts,
        ac_cv_prog_ld_version_script,
        [ac_cv_prog_ld_version_script=no
         echo "TEST { local: *; };" > conftest.ver
         LDFLAGS="$LDFLAGS -Wl,--version-script,conftest.ver"
         _AC_LINK_IFELSE([AC_LANG_PROGRAM()],
                [ac_cv_prog_ld_version_script=yes], [])
        ])
LDFLAGS=$ac_save_LDFLAGS
AM_CONDITIONAL([HAVE_VERSIONING], [test x$ac_cv_prog_ld_version_script = xyes])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
