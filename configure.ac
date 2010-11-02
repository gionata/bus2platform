AC_PREREQ([2.67])
AC_INIT([gap_graph], [20101020], [massi@diiga.univpm.it])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/Bus.cpp])
AC_CONFIG_HEADERS([config.h])

# store current user given compiler flags to avoid default setup via AC_PROG_CXX
OLD_CXXFLAGS=$CXXFLAGS
# check for C++ preprocessor and compiler and the library compiler
# (might change the compiler flags)
AC_PROG_CXXCPP
AC_PROG_CXX
# reset compiler flags to initial flags
CXXFLAGS=$OLD_CXXFLAGS

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcairo':
AC_CHECK_LIB([cairo], [main])
# FIXME: Replace `main' with a function in `-llpsolve55':
AC_CHECK_LIB([lpsolve55], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

AC_CHECK_FUNCS([sqrt])

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])

######################################################################
# DOXYGEN SUPPORT
######################################################################

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[Doxyfile])

######################################################################

AC_OUTPUT


