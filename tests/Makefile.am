
#Common options
AM_CFLAGS = -I$(top_srcdir) $(MMC_CFLAGS)
common_libadd = ../ssc/libssc.la -lm $(MMC_LIBS) 

#Test support library
check_LTLIBRARIES = libtest.la
libtest_la_SOURCES = libtest.c libtest.h
libtest_la_LIBADD = $(common_libadd)

#Test programs
check_PROGRAMS = proto_int

proto_int_SOURCES = proto_int_src/idl.c proto_int_src/idl.h proto_int_src/main.c

LDADD = libtest.la $(common_libadd)

%/idl.c %/idl.h: %/idl.txt $(top_builddir)/sidc/sidc
	$(top_builddir)/sidc/sidc $< $*/idl

#Tests to run
TESTS = $(check_PROGRAMS)
