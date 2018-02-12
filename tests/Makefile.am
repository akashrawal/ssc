
BUILT_SOURCES = proto_int/idl.txt.c proto_int/idl.txt.h

check_PROGRAMS = proto_int

proto_int_SOURCES = proto_int/idl.txt.c proto_int/idl.txt.h proto_int/main.c

AM_CFLAGS = -I$(top_srcdir)

%.txt.c %.txt.h: %.txt
	$(top_builddir)/sidc/sidc $< $<
