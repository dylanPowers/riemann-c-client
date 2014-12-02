# -- Global settings --
AM_MAKEFLAGS			= --no-print-directory
AM_CFLAGS			= -I${top_srcdir}/lib -I${top_builddir}/lib
ACLOCAL_AMFLAGS			= -Im4

LDADD				= lib/libriemann-client.la

pkgincludedir			= ${includedir}/riemann
protodir			= ${pkgincludedir}/proto

VERSION				= @VERSION@${SNAPSHOT_VERSION}

# -- Libraries --
LRC_CURRENT			= 3
LRC_REVISION			= 0
LRC_AGE				= 3

lib_LTLIBRARIES			= lib/libriemann-client.la

lib_libriemann_client_la_CFLAGS	= ${AM_CFLAGS} ${PROTOBUF_C_CFLAGS}
lib_libriemann_client_la_LDFLAGS= \
	${PROTOBUF_C_LDFLAGS}	  \
	-version-info ${LRC_CURRENT}:${LRC_REVISION}:${LRC_AGE}
lib_libriemann_client_la_LIBADD	= -lprotobuf-c
proto_files			= \
	lib/riemann/proto/riemann.pb-c.c  \
	lib/riemann/proto/riemann.pb-c.h
nodist_proto_HEADERS		= \
	lib/riemann/proto/riemann.pb-c.h
pkginclude_HEADERS = \
	lib/riemann/client.h	  \
	lib/riemann/event.h	  \
	lib/riemann/message.h	  \
	lib/riemann/attribute.h	  \
	lib/riemann/query.h	  \
	lib/riemann/simple.h	  \
	lib/riemann/riemann-client.h
lib_libriemann_client_la_SOURCES= \
	lib/riemann/client.c	  \
	lib/riemann/event.c	  \
	lib/riemann/message.c	  \
	lib/riemann/attribute.c	  \
	lib/riemann/query.c	  \
	lib/riemann/simple.c
$(am_lib_libriemann_client_la_OBJECTS): ${proto_files}

nodist_lib_libriemann_client_la_SOURCES	= \
	${proto_files}

CLEANFILES			= ${proto_files}

${proto_files}: ${top_srcdir}/lib/riemann/proto/riemann.proto
	${AM_V_at} ${mkinstalldirs} ${top_builddir}/lib/riemann/proto
	${AM_V_GEN} protoc-c $^ -I${top_srcdir}/lib/riemann/proto --c_out=${top_builddir}/lib/riemann/proto

if HAVE_VERSIONING
lib_libriemann_client_la_LDFLAGS += \
	-Wl,--version-script,$(top_srcdir)/lib/riemann/libriemann.ver
lib_libriemann_client_la_DEPENDENCIES	= ${top_srcdir}/lib/riemann/libriemann.ver
endif

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= lib/riemann/riemann-client.pc

# -- Testcases --
if HAVE_CHECK
UNIT_TESTS			= tests/check_libriemann
TESTS				= ${UNIT_TESTS}

TESTS_ENVIRONMENT		= CK_FORK=no

tests/check_%: CFLAGS += ${CHECK_CFLAGS}
tests/check_%: LDADD += ${CHECK_LIBS} -ldl
tests/check_%: LDFLAGS += -no-install

check_PROGRAMS			= ${TESTS}
endif

check_libriemann_srcs		= \
	tests/mocks.h		  \
	tests/mocks.c		  \
	tests/tests.h		  \
	tests/check_library.c	  \
	tests/check_events.c	  \
	tests/check_messages.c	  \
	tests/check_client.c	  \
	tests/check_attributes.c  \
	tests/check_queries.c	  \
	tests/check_simple.c	  \
	tests/check_libriemann.c

# -- Binaries --
bin_PROGRAMS			= \
	src/riemann-client

src_riemann_client_CFLAGS	= $(AM_CFLAGS) $(JSON_C_CFLAGS)
src_riemann_client_LDADD	= $(JSON_C_LIBS) $(LDADD)
src_riemann_client_EXTRA_DIST	= \
	src/cmd-send.c		  \
	src/cmd-query.c

# -- Extra files to distribute --
EXTRA_DIST			= README.md NEWS GPL LGPL \
				  lib/riemann/proto/riemann.proto \
				  lib/riemann/libriemann.ver \
				  lib/riemann/riemann-client.pc \
				  lib/riemann/_private.h \
				  ${check_libriemann_srcs} \
				  src/riemann-client.ronn \
				  ${src_riemann_client_EXTRA_DIST} \
				  docs/API.md

# -- Manual pages --
if HAVE_RONN
CLEANFILES			+= src/riemann-client.1

man1_MANS			 = \
	src/riemann-client.1

src/riemann-client.1: DATE=$(shell grep '^\*' ${top_srcdir}/NEWS | \
				   head -n 1 | sed -e "s,^.*<\([^ ]*\).*,\1,")
src/riemann-client.1: src/riemann-client.ronn NEWS
	${AM_V_at} ${mkinstalldirs} ${top_builddir}/src
	$(AM_V_GEN)ronn --manual "riemann-c-client" \
			--organization "The MadHouse Project" \
			--roff \
			--pipe \
			--date "${DATE}" $< >$@

all: ${man1_MANS}
endif

# -- Custom targets --
coverage: coverage.info.html

coverage.info.html: coverage.info
	$(AM_V_GEN)genhtml --quiet $^ --output-directory $@

coverage.info: check
	$(AM_V_GEN)lcov --quiet --capture --directory ${top_builddir}/lib --output $@ -b ${top_builddir} && \
		   lcov --quiet --remove $@ '*/lib/riemann/proto/*' -o $@

.PHONY: coverage
CLEANFILES			+= coverage.info

clean-local:
	rm -rf coverage.info.html
	find ${top_builddir} -name '*.gcda' -or -name '*.gcno' | xargs rm -f
