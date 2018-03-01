
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
SUBDIRS = proto_int \
          test_int \
          test_string \
		  test_optional \
		  test_seq \
		  test_optional_ptr

TESTS = proto_int/main$(EXEEXT) \
        test_int/idl.txt            test_int/main$(EXEEXT) \
		test_string/idl.txt         test_string/main$(EXEEXT) \
        test_optional/idl.txt       test_optional/main$(EXEEXT) \
        test_seq/idl.txt            test_seq/main$(EXEEXT) \
        test_optional_ptr/idl.txt   test_optional_ptr/main$(EXEEXT)

LOG_COMPILER = sh $(builddir)/logcc.sh

