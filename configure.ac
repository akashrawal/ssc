#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([SSC],[0.0.0],[],[ssc],[https://github.com/akashrawal/ssc])
AC_CONFIG_SRCDIR([ssc/ssc.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE([-Wall -Werror silent-rules subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AM_PROG_AR
LT_INIT

# Checks for programs.
#TODO: Enforce C11
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([MMC], [mmc >= 0.0.0])

#Check for endianness
AC_C_BIGENDIAN([AC_DEFINE([SSC_UINT_BIG_ENDIAN], [1],
			       [Not useful here, please refer ssc/generated.h])],
               [AC_DEFINE([SSC_UINT_LITTLE_ENDIAN], [1],
			       [Not useful here, please refer ssc/generated.h])],
			   [AC_DEFINE([SSC_UINT_UNKNOWN_ENDIAN], [1],
			       [Not useful here, please refer ssc/generated.h])],
			   [AC_DEFINE([SSC_UINT_UNKNOWN_ENDIAN], [1],
			       [Not useful here, please refer ssc/generated.h])])


#Write all output
AC_CONFIG_FILES([Makefile
                 data/Makefile
                 data/ssc.pc
                 ssc/Makefile
                 sidc/Makefile
				 tests/Makefile
				 tests/logcc.sh
				 tests/proto_int/Makefile
				 tests/test_int/Makefile
				 tests/test_string/Makefile
				 tests/test_optional/Makefile
				 tests/test_seq/Makefile
				 tests/test_optional_ptr/Makefile
				 tests/test_iface_1/Makefile
				 tests/test_iface_2/Makefile
				 ])
AC_CONFIG_HEADERS([config.h ssc/generated.h])
AC_OUTPUT
