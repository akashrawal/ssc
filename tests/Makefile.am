
#Common
AM_CFLAGS = -I$(top_srcdir) $(MMC_CFLAGS)
LDADD = ../ssc/libssc.la -lm $(MMC_LIBS) 


#Test support library
BUILT_SOURCES = libtest.la
noinst_LTLIBRARIES = libtest.la
libtest_la_SOURCES = libtest.c libtest.h
libtest_la_LIBADD = $(LDADD)

#Log compiler
nodist_check_SCRIPTS = logcc.sh
DISTCLEANFILES = logcc.sh
EXTRA_DIST = logcc.sh.in

#Unit tests
check_PROGRAMS = lib_private


#Tests to run
SUBDIRS = proto_int \
          test_int \
          test_string \
		  test_optional \
		  test_seq \
		  test_optional_ptr

TESTS = $(check_PROGRAMS) \
        proto_int/main$(EXEEXT) \
        test_int/idl.txt            test_int/main$(EXEEXT) \
		test_string/idl.txt         test_string/main$(EXEEXT) \
        test_optional/idl.txt       test_optional/main$(EXEEXT) \
        test_seq/idl.txt            test_seq/main$(EXEEXT) \
        test_optional_ptr/idl.txt   test_optional_ptr/main$(EXEEXT)

LOG_COMPILER = sh $(builddir)/logcc.sh

