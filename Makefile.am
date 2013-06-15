# -- Global settings --
AM_MAKEFLAGS			= --no-print-directory
AM_CFLAGS			= -I${top_srcdir}/lib

includedir			= ${prefix}/include/riemann

# -- Libraries --
LRC_CURRENT			= 0
LRC_REVISION			= 0
LRC_AGE				= 0

lib_LTLIBRARIES			= lib/libriemann-client.la

lib_libriemann_client_la_LDFLAGS= -version-info ${LRC_CURRENT}:${LRC_REVISION}:${LRC_AGE}
lib_libriemann_client_la_SOURCES= \
	lib/client.c		  \
	lib/riemann/client.h

nobase_include_HEADERS= \
	lib/riemann/client.h

if HAVE_VERSIONING
lib_libriemann_client_la_LDFLAGS += \
	-Wl,--version-script,$(top_srcdir)/lib/libriemann.ver
endif

# -- Testcases --
UNIT_TESTS			= tests/check_library
TESTS				= ${UNIT_TESTS}

test_cflags			= ${AM_CFLAGS} ${CHECK_CFLAGS}
test_ldadd			= ${CHECK_LIBS} lib/libriemann-client.la

tests_check_library_CFLAGS	= ${test_cflags}
tests_check_library_LDADD	= ${test_ldadd}

check_PROGRAMS			= ${TESTS}

# -- Extra files to distribute --
EXTRA_DIST			= README.md GPL LGPL
