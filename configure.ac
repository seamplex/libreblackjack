dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

m4_ifndef([_VERSION_M4_INCLUDED], [
  m4_define([_VERSION_M4_INCLUDED], 1)
  m4_include([version.m4])
])

AC_INIT([blackjack],[libreblackjackversion],[jeremy@seamplex.com])
AM_INIT_AUTOMAKE
# AM_GNU_GETTEXT([external])

AC_CONFIG_SRCDIR([src/main.cpp])

AC_CANONICAL_HOST
AC_PROG_CXX

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [Compile with debugging symbols])],
  [enable_debug=$enableval], [enable_debug=no])

######################
# default optimization flags
AS_IF([test "x$CXXFLAGS" = "x-g -O2"],
  AS_IF([test "x$with_debug" = "xyes" -o "x$enable_debug" = "xyes"] , [
    CXXFLAGS="-g -O0"
    AC_DEFINE([DEBUG], [1], [Define to 1 if debugging is enabled])
  ],[
    CXXFLAGS="-O3 -flto=auto"
  ])
)

AC_CHECK_HEADER([readline/readline.h])
AC_CHECK_LIB([readline], [readline])



AC_MSG_NOTICE([creating version-conf.h])
cat << EOF > src/version-conf.h
#define COMPILATION_ARCH     "${host_os} ${host_cpu}"
#define COMPILER_VERSION     "`$CXX --version | head -n1`"
#define COMPILER_CXXFLAGS    "$CXXFLAGS"
EOF

#AC_CONFIG_FILES([Makefile doc/Makefile po/Makefile.in])
AC_CONFIG_FILES([Makefile doc/Makefile])
AC_OUTPUT
