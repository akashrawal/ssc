
lib_LTLIBRARIES = libssc.la

#Generated file generated.h
DISTCLEANFILES = generated.h
EXTRA_DIST = generated.h.in

#libssc.la
ssc_c =  \
	types.c \
	serialize.c \
	interface.c \
	msg.c

ssc_h =  ssc.h incl.h \
	types.h \
	serialize.h \
	interface.h \
	msg.h
	
     
libssc_la_SOURCES = $(ssc_c) $(ssc_h)       
	        
nodist_libssc_la_SOURCES = generated.h
libssc_la_CFLAGS = -Wall -I$(top_builddir) -I$(top_srcdir)
libssc_la_LIBADD = -lm $(MMC_LIBS)

#Headers
sscincludedir = $(includedir)/ssc
sscinclude_HEADERS = $(ssc_h)                
nodist_sscinclude_HEADERS = generated.h    
