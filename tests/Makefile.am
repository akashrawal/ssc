
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
LOG_COMPILER = sh $(builddir)/logcc.sh

#Unit tests
check_PROGRAMS = 


#Tests to run
SUBDIRS = proto_int \
          test_int \
          test_string \
		  test_optional \
		  test_seq \
		  test_optional_ptr \
		  test_iface_1 \
		  test_iface_2

TESTS = $(check_PROGRAMS) \
        proto_int/main$(EXEEXT) \
        test_int/idl.txt            test_int/main$(EXEEXT) \
		test_string/idl.txt         test_string/main$(EXEEXT) \
        test_optional/idl.txt       test_optional/main$(EXEEXT) \
        test_seq/idl.txt            test_seq/main$(EXEEXT) \
        test_optional_ptr/idl.txt   test_optional_ptr/main$(EXEEXT) \
        test_iface_1/idl.txt        test_iface_1/main$(EXEEXT) \
        test_iface_2/idl.txt        test_iface_2/main$(EXEEXT)


