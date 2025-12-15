#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([min12xxw], [0.0.9], [mala@hinterbergen.de])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([min12xxw.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB(intl, gettext)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h stddef.h inttypes.h stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strcasecmp strchr getline fgetln gettext])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
