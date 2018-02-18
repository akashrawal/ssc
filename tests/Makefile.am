
#BUILT_SOURCES = proto_int/idl.txt.c proto_int/idl.txt.h

check_PROGRAMS = proto_int

proto_int_SOURCES = proto_int_src/idl.c proto_int_src/idl.h proto_int_src/main.c

AM_CFLAGS = -I$(top_srcdir)
LDADD = ../ssc/libssc.la -lm $(MMC_LIBS)

%/idl.c %/idl.h: %/idl.txt $(top_builddir)/sidc/sidc
	$(top_builddir)/sidc/sidc $< $*/idl

TESTS = $(check_PROGRAMS)
