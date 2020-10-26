dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

m4_include([version.m4])
AC_INIT([blackjack],[libreblackjackversion],[jeremy@seamplex.com])
AM_INIT_AUTOMAKE
# AM_GNU_GETTEXT([external])

AC_CONFIG_SRCDIR([src/main.cpp])

AC_CANONICAL_HOST
AC_PROG_CXX

AC_CHECK_HEADER([readline/readline.h])
AC_CHECK_LIB([readline], [readline])

AC_MSG_NOTICE([creating version-conf.h])
cat << EOF > src/version-conf.h
#define COMPILATION_ARCH     "${host_os} ${host_cpu}"
#define COMPILER_VERSION     "`$CXX --version | head -n1`"
#define COMPILER_CFLAGS      "$CFLAGS"
EOF

#AC_CONFIG_FILES([Makefile doc/Makefile po/Makefile.in])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
