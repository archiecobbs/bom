#
# bom - Deals with Unicode byte order marks
#
# Copyright (C) 2021 Archie L. Cobbs. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

AC_INIT([bom Deals with Unicode byte order marks], [1.0.0], [https://github.com/archiecobbs/bom/], [bom])
AC_CONFIG_AUX_DIR(scripts)
AM_INIT_AUTOMAKE
dnl AM_MAINTAINER_MODE
AC_PREREQ(2.59)
AC_PREFIX_DEFAULT(/usr)
AC_PROG_MAKE_SET

[CFLAGS="-g -O3 -pipe -Wall -Waggregate-return -Wcast-align -Wchar-subscripts -Wcomment -Wformat -Wimplicit -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wno-long-long -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wswitch -Wtrigraphs -Wuninitialized -Wunused -Wwrite-strings -Wshadow -Wstrict-prototypes -Wcast-qual $CFLAGS"]
AC_SUBST(CFLAGS)

# Compile flags for Linux
AC_DEFINE(_DEFAULT_SOURCE, 1, Default functions)
AC_DEFINE(_GNU_SOURCE, 1, GNU functions)
AC_DEFINE(_BSD_SOURCE, 1, BSD functions)
AC_DEFINE(_XOPEN_SOURCE, 500, XOpen functions)

# Compile flags for Mac OS
AC_DEFINE(_DARWIN_C_SOURCE, 1, MacOS functions)

# Check for required programs
AC_PROG_INSTALL
AC_PROG_CC
AC_PATH_PROG([CAT], [cat], [], [])
if test "x${CAT}" = "x"; then
    AC_MSG_ERROR[cat not found]
fi
AC_PATH_PROG([SED], [sed], [], [])
if test "x${SED}" = "x"; then
    AC_MSG_ERROR[sed not found]
fi

# Check for required libc functions
AC_SEARCH_LIBS([iconv_open], [iconv],,
    [if test `uname -o` = 'Cygwin' -a -f /usr/lib/libiconv.a; then LIBS="-liconv ${LIBS}"; else AC_MSG_ERROR([required function iconv_open missing]); fi])

# Check for required header files
AC_HEADER_STDC
AC_CHECK_HEADERS(ctype.h errno.h stdio.h stdlib.h string.h unistd.h sys/stat.h sys/types.h, [],
        [AC_MSG_ERROR([required header file '$ac_header' missing])])

# Optional features
AC_ARG_ENABLE(Werror,
    AC_HELP_STRING([--enable-Werror],
        [enable compilation with -Werror flag (default NO)]),
    [test x"$enableval" = "xyes" && CFLAGS="${CFLAGS} -Werror"])

# Generated files
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(bom.1)
AM_CONFIG_HEADER(config.h)

# Go
AC_OUTPUT
