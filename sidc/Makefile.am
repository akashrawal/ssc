

gen = lex.yy.c lex.yy.h parser.tab.c parser.tab.h

bin_PROGRAMS = sidc

sidc_SOURCES = incl.h \
	bst.c         bst.h \
	symbol.c      symbol.h \
	parselib.c    parselib.h \
	codegen.c     codegen.h \
	structure.c   structure.h \
	sequencer.c   sequencer.h \
	main.c
	

nodist_sidc_SOURCES = $(gen)
CLEANFILES = $(gen)
BUILT_SOURCES = $(gen)

lex.yy.c lex.yy.h: lex.l
	flex $(srcdir)/lex.l

parser.tab.c parser.tab.h : parser.y
	bison $(srcdir)/parser.y

EXTRA_DIST = lex.l parser.y

sidc_CFLAGS = -Wall -I$(top_builddir) -I$(top_srcdir) 
sidc_LDADD = $(top_builddir)/ssc/libssc.la -lm $(MMC_LIBS)
