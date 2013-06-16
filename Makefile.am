# -- Global settings --
AM_MAKEFLAGS			= --no-print-directory
AM_CFLAGS			= -I${top_srcdir}/lib -I${top_builddir}/lib
ACLOCAL_AMFLAGS			= -Im4

includedir			= ${prefix}/include/riemann

# -- Libraries --
LRC_CURRENT			= 0
LRC_REVISION			= 0
LRC_AGE				= 0

lib_LTLIBRARIES			= lib/libriemann-client.la

lib_libriemann_client_la_CFLAGS	= ${AM_CFLAGS} ${PROTOBUF_C_CFLAGS}
lib_libriemann_client_la_LDFLAGS= \
	${PROTOBUF_C_LDFLAGS}	  \
	-version-info ${LRC_CURRENT}:${LRC_REVISION}:${LRC_AGE}
lib_libriemann_client_la_LIBADD	= -lprotobuf-c
lib_libriemann_client_la_SOURCES= \
	lib/client.c		  \
	lib/riemann/client.h	  \
	lib/event.c		  \
	lib/riemann/event.h	  \
	lib/message.c		  \
	lib/riemann/message.h	  \
	lib/attribute.c		  \
	lib/riemann/attribute.h	  \
	lib/riemann/proto/riemann.pb-c.c  \
	lib/riemann/proto/riemann.pb-c.h

proto_files			= \
	lib/riemann/proto/riemann.pb-c.c  \
	lib/riemann/proto/riemann.pb-c.h

BUILT_SOURCES			= ${proto_files}
CLEANFILES			= ${proto_files}

${BUILT_SOURCES}: ${top_srcdir}/lib/proto/riemann.proto
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
UNIT_TESTS			= tests/check_libriemann
TESTS				= ${UNIT_TESTS}

tests/check_%: CFLAGS += ${CHECK_CFLAGS}
tests/check_%: LDADD += ${CHECK_LIBS} lib/libriemann-client.la

check_PROGRAMS			= ${TESTS}

check_libriemann_srcs		= \
	tests/check_library.c	  \
	tests/check_events.c	  \
	tests/check_messages.c	  \
	tests/check_client.c	  \
	tests/check_attribute.c

# -- Binaries --
bin_PROGRAMS			= src/riemann-client

src_riemann_client_LDADD	= lib/libriemann-client.la

# -- Extra files to distribute --
EXTRA_DIST			= README.md GPL LGPL \
				  lib/proto/riemann.proto \
				  lib/libriemann.ver \
				  tests/tests.h \
				  ${check_libriemann_srcs}
