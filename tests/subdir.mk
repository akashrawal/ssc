#Test program
check_PROGRAMS = main
main_SOURCES = main.c
nodist_main_SOURCES = idl.c idl.h
AM_CFLAGS = -I$(top_srcdir) $(MMC_CFLAGS)
LDADD = ../libtest.la ../../ssc/libssc.la -lm $(MMC_LIBS) 
main.$(OBJEXT): idl.h

#IDL
idl.c idl.h: idl.txt $(top_builddir)/sidc/sidc
	$(top_builddir)/sidc/sidc $(srcdir)/idl.txt $(builddir)/idl
EXTRA_DIST = idl.txt
CLEANFILES = idl.c idl.h
