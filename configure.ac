
# Process this file with autoconf to produce a configure script.

AC_INIT([reposit], [1.8.0],
        [quantlib-dev@lists.sourceforge.net],
        [reposit])
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([rp/reposit.hpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([rp/config.hpp])
AM_INIT_AUTOMAKE([foreign])

AC_COPYRIGHT([
Copyright (C) 2004, 2005, 2006, 2007 Eric Ehlers

Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
Copyright (C) 2005, 2006 Plamen Neykov
Copyright (C) 2004 StatPro Italia srl

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it under the
terms of the QuantLib license.  You should have received a copy of the
license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.])

AC_PROG_INSTALL

# Default compilation variables

AC_SUBST([CXXFLAGS],[${CXXFLAGS='-g -O2 -Wall'}])

# Continue setup

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_LANG([C++])

# Confirm existence of dependencies

# Configure logging

AC_ARG_WITH([logging],
    [AS_HELP_STRING([--without-logging], [disable support for logging, write log messages to stdout])],
    [],
    [with_logging=yes])
AM_CONDITIONAL([RP_LINK_BOOST_LOGGING], [test "x$with_logging" != xno])
AS_IF([test "x$with_logging" != xno], AC_DEFINE([RP_INCLUDE_BOOST_LOGGING], [1], [Support for logging]))

# Check for tools needed for building documentation

AC_PATH_PROG([DOXYGEN], [doxygen])
AC_PATH_PROG([SED], [sed])

# Done, output the configured files

AC_CONFIG_FILES([
    Docs/Makefile
    Examples/C++/Makefile
    Examples/ExampleObjects/Enumerations/Makefile
    Examples/ExampleObjects/Library/Makefile
    Examples/ExampleObjects/Makefile
    Examples/ExampleObjects/Objects/Makefile
    Examples/ExampleObjects/Serialization/Makefile
    Examples/ExampleObjects/ValueObjects/Makefile
    Examples/Makefile
    Examples/xl/Makefile
    Makefile
    rp/conversions/Makefile
    rp/enumerations/Makefile
    rp/loop/Makefile
    rp/Makefile
    rp/valueobjects/Makefile
    rpxl/Makefile
    xlsdk/Makefile
    xlsdk/x64/Makefile ])
AC_OUTPUT

