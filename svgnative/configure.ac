#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([include/SVGDocument.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AM_INIT_AUTOMAKE([1.7 foreign])

#############################################################
# SVGNativeViwerLib version
SVGNATIVEVIEWERLIB_VERSION=9.0.9
SVGNATIVEVIEWERLIB_VERSION=`echo ${SVGNATIVEVIEWERLIB_VERSION} | tr "." ":"`
AC_SUBST(SVGNATIVEVIEWERLIB_VERSION)

#############################################################
# Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)
AC_PROG_CC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

#############################################################
# Checks for pkg-config.
PKG_PROG_PKG_CONFIG

# Following check is not needed because we request C++11.
##############################################################
## Checks for header files.
##
#AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h])
#
## Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
#AC_C_INLINE
#AC_TYPE_INT32_T
#AC_TYPE_SIZE_T
#AC_TYPE_UINT32_T
#AC_TYPE_UINT8_T
#
##############################################################
## Checks for library functions.
##
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([bzero memset pow sqrt])

#############################################################
# Checks for libraries.
#
# Boost -----------------------------------------------------

  AX_BOOST_BASE([1.55],[
    if test "x${cross_compiling}" = "xyes"
    then
      case "${BOOST_CPPFLAGS}" in
      -I/usr/include | -I/usr/include/* | -I/usr/local/include | -I/usr/local/include/* )
        AC_MSG_ERROR([Found boost might be for host (${BOOST_CPPFLAGS}), but we are cross-compiling]) 
        ;;
      esac
    fi
  ],[
    AC_MSG_ERROR([Boost >= 1.55 is needed])
  ])

# Text ------------------------------------------------------

  AC_ARG_ENABLE(port-text,
                AC_HELP_STRING([--disable-port-text],
                               [Don't build the text port.]),
                enable_port_text=$enableval,
                enable_port_text="yes")
  AC_ARG_ENABLE(port-text-example,
                AC_HELP_STRING([--disable-port-text-example],
                               [Don't build example program for the text port.]),
                enable_port_text_example=$enableval,
                enable_port_text_example=${enable_port_text})

  AM_CONDITIONAL(ENABLE_PORT_TEXT, test "x${enable_port_text}" = "xyes")
  AM_CONDITIONAL(ENABLE_PORT_TEXT_EXAMPLE, test "x${enable_port_text_example}" = "xyes")

# CoreGraphics ----------------------------------------------

  AC_ARG_ENABLE(port-skia,
                AC_HELP_STRING([--disable-port-coregraphics],
                               [Don't build the coregraphics port.]),
                enable_port_coregraphics=$enableval,
                enable_port_coregraphics="no")
  AC_ARG_ENABLE(port-coregraphics-example,
                AC_HELP_STRING([--disable-port-coregraphics-example],
                               [Don't build example program for the coregraphics port.]),
                enable_port_coregraphics_example=$enableval,
                enable_port_coregraphics_example=${enable_port_coregraphics})

  AM_CONDITIONAL(ENABLE_PORT_COREGRAPHICS, test "x${enable_port_coregraphics}" = "xyes")
  AM_CONDITIONAL(ENABLE_PORT_COREGRAPHICS_EXAMPLE, test "x${enable_port_coregraphics_example}" = "xyes")

# Skia ------------------------------------------------------

  AC_ARG_ENABLE(port-skia,
                AC_HELP_STRING([--disable-port-skia],
                               [Don't build the skia port.]),
                enable_port_skia=$enableval,
                enable_port_skia="no")
  AC_ARG_ENABLE(port-skia-example,
                AC_HELP_STRING([--disable-port-skia-example],
                               [Don't build example program for the skia port.]),
                enable_port_skia_example=$enableval,
                enable_port_skia_example=${enable_port_skia})

  AM_CONDITIONAL(ENABLE_PORT_SKIA, test "x${enable_port_skia}" = "xyes")
  AM_CONDITIONAL(ENABLE_PORT_SKIA_EXAMPLE, test "x${enable_port_skia_example}" = "xyes")

# GDI+ ------------------------------------------------------

  AC_ARG_ENABLE(port-gdiplus,
                AC_HELP_STRING([--disable-port-gdiplus],
                               [Don't build the gdiplus port.]),
                enable_port_gdiplus=$enableval,
                enable_port_gdiplus="no")
  AC_ARG_ENABLE(port-gdiplus-example,
                AC_HELP_STRING([--disable-port-gdiplus-example],
                               [Don't build example program for the gdiplus port.]),
                enable_port_gdiplus_example=$enableval,
                enable_port_gdiplus_example=${enable_port_gdiplus})

  # TBD: check MinGW / WINE headers...
  AM_CONDITIONAL(ENABLE_PORT_GDIPLUS, test "x${enable_port_gdiplus}" = "xyes")
  AM_CONDITIONAL(ENABLE_PORT_GDIPLUS_EXAMPLE, test "x${enable_port_gdiplus_example}" = "xyes")

# Cairo -----------------------------------------------------
  CAIRO_VERSION="1.10.0"
  AC_SUBST(CAIRO_VERSION)
  CAIRO_DEPS="cairo >= $CAIRO_VERSION cairo-ft >= $CAIRO_VERSION"
  AC_ARG_ENABLE(port-cairo,
                AC_HELP_STRING([--disable-port-cairo],
                               [Don't build the cairo port.]),
                enable_port_cairo=$enableval,
                enable_port_cairo="try")
  use_cairo=""
  if test x$enable_port_cairo = xyes; then
    PKG_CHECK_MODULES(CAIRO, $CAIRO_DEPS)
  elif test x$enable_port_cairo = xtry; then
    PKG_CHECK_MODULES(CAIRO, $CAIRO_DEPS,
                      [enable_port_cairo="yes"],
                      [enable_port_cairo="no"
                       use_cairo="no (requires cairo >= $CAIRO_VERSION)"])
  fi
  if test "x$use_cairo" = "x"; then
     use_cairo=$enable_port_cairo
  fi

  AC_ARG_ENABLE(port-cairo-example,
                AC_HELP_STRING([--disable-port-cairo-example],
                               [Don't build example program for the cairo port.]),
                enable_port_cairo_example=$enableval,
                enable_port_cairo_example=${enable_port_cairo})


  AC_SUBST(CAIRO_CFLAGS)
  AC_SUBST(CAIRO_LIBS)
  AM_CONDITIONAL(ENABLE_PORT_CAIRO, test "x${enable_port_cairo}" = "xyes")
  AM_CONDITIONAL(ENABLE_PORT_CAIRO_EXAMPLE, test "x${enable_port_cairo_example}" = "xyes")

# Qt --------------------------------------------------------
# TBD

AC_OUTPUT
