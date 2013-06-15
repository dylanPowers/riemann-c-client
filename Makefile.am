# -- Global settings --
AM_MAKEFLAGS			= --no-print-directory
AM_CFLAGS			= -I${top_srcdir}/lib -I${top_builddir}/lib

includedir			= ${prefix}/include/riemann

# -- Libraries --
LRC_CURRENT			= 0
LRC_REVISION			= 0
LRC_AGE				= 0

lib_LTLIBRARIES			= lib/libriemann-client.la

lib_libriemann_client_la_LDFLAGS= -version-info ${LRC_CURRENT}:${LRC_REVISION}:${LRC_AGE}
lib_libriemann_client_la_LIBADD	= -lprotobuf-c
lib_libriemann_client_la_SOURCES= \
	lib/client.c		  \
	lib/riemann/client.h	  \
	lib/event.c		  \
	lib/riemann/event.h	  \
	lib/message.c		  \
	lib/riemann/message.h	  \
	lib/riemann/proto/riemann.pb-c.c  \
	lib/riemann/proto/riemann.pb-c.h

proto_files			= \
	lib/riemann/proto/riemann.pb-c.c  \
	lib/riemann/proto/riemann.pb-c.h

CLEANFILES			= ${proto_files}

${proto_files}: ${top_srcdir}/lib/proto/riemann.proto
	${AM_V_at} ${mkinstalldirs} ${top_builddir}/lib/riemann/proto
	${AM_V_GEN} protoc-c $^ -I${top_srcdir}/lib/proto --c_out=${top_builddir}/lib/riemann/proto

nobase_include_HEADERS= \
	lib/riemann/client.h

if HAVE_VERSIONING
lib_libriemann_client_la_LDFLAGS += \
	-Wl,--version-script,$(top_srcdir)/lib/libriemann.ver
lib_libriemann_client_la_DEPENDENCIES	= ${top_srcdir}/lib/libriemann.ver
endif

# -- Testcases --
UNIT_TESTS			= \
	tests/check_library	  \
	tests/check_events	  \
	tests/check_messages
TESTS				= ${UNIT_TESTS}

tests/check_%: CFLAGS += ${CHECK_CFLAGS}
tests/check_%: LDADD += ${CHECK_LIBS} lib/libriemann-client.la

check_PROGRAMS			= ${TESTS}

# -- Extra files to distribute --
EXTRA_DIST			= README.md GPL LGPL \
				  lib/proto/riemann.proto \
				  lib/libriemann.ver \
				  tests/tests.h
