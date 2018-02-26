
#Test support library
BUILT_SOURCES = libtest.la
noinst_LTLIBRARIES = libtest.la
libtest_la_SOURCES = libtest.c libtest.h
AM_CFLAGS = -I$(top_srcdir) $(MMC_CFLAGS)
libtest_la_LIBADD = ../ssc/libssc.la -lm $(MMC_LIBS) 

#Log compiler
nodist_check_SCRIPTS = logcc.sh
DISTCLEANFILES = logcc.sh
EXTRA_DIST = logcc.sh.in

#Tests to run
SUBDIRS = proto_int
TESTS = proto_int/main$(EXEEXT) proto_int/idl.txt
LOG_COMPILER = sh $(builddir)/logcc.sh

