
bin_PROGRAMS = sidc

sidc_SOURCES = incl.h \
	bst.c         bst.h

sidc_CFLAGS = -Wall -I$(top_builddir) -I$(top_srcdir) 
sidc_LDADD = $(top_builddir)/ssc/libssc.la -lm $(MMC_LIBS)
