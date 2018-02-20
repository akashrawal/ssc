
#Test support library
BUILT_SOURCES = libtest.la
noinst_LTLIBRARIES = libtest.la
libtest_la_SOURCES = libtest.c libtest.h
AM_CFLAGS = -I$(top_srcdir) $(MMC_CFLAGS)
libtest_la_LIBADD = ../ssc/libssc.la -lm $(MMC_LIBS) 

#Tests to run
TESTS = proto_int/main$(EXEEXT)
SUBDIRS = proto_int
