
lib_LTLIBRARIES = libssc.la

#Generated file(s)
generated.h: generator.sh
	$(srcdir)/generator.sh
	
CLEANFILES = generated.h
BUILT_SOURCES = generated.h

#libssc.la
ssc_c =  \
	 

ssc_h =  \
	
     
libssc_la_SOURCES = $(ssc_c) $(ssc_h)       
	        
nodist_libssc_la_SOURCES = generated.h
libssc_la_CFLAGS = -Wall -I$(top_builddir) -I$(top_srcdir)
libssc_la_LDFLAGS = -lm

#Headers
sscincludedir = $(includedir)/ssc
sscinclude_HEADERS = $(ssc_h)                
                        

nodist_sscinclude_HEADERS = generated.h    

#Distribute the generator and configuration file
EXTRA_DIST = config.sh.in generator.sh
