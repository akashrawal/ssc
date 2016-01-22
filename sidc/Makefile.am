
bin_PROGRAMS = sdlc

sdlc_SOURCES = incl.h \
	bst.c         bst.h

sdlc_CFLAGS = -Wall -I$(top_builddir) -I$(top_srcdir) 
sdlc_LDADD = $(top_builddir)/ssc/libssc.la -lm $(MMC_LIBS)
