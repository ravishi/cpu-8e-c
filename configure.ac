#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cpu-8e], [0.0.1], [dirleyrls@gmail.com])
#AC_CONFIG_SRCDIR([cpu-8e/main.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11.6 foreign -Wall -Werror dist-bzip2])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

AC_CONFIG_FILES([
    Makefile
    cpu-8e/Makefile
    disasm/Makefile
    lib8e/Makefile
])

AC_OUTPUT
