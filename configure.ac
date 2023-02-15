#AC_PREREQ([2.71])
AC_INIT([wxpolygon],[1.2])

# MINGW_AC_WIN32_NATIVE_HOST
# --------------------------
# Check if the runtime platform is a native Win32 host.
#
AC_DEFUN([MINGW_AC_WIN32_NATIVE_HOST],
[AC_CACHE_CHECK([whether we are building for a Win32 host], 
                [mingw_cv_win32_host],
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#ifdef _WIN32
choke me
#endif
])], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))])


AC_LANG([C++])
: ${CXXFLAGS=""}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

MINGW_AC_WIN32_NATIVE_HOST

#Process enables:

AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [enables debug compilation with -g, default is no debug switch])
)
if test "$enable_debug" == "yes"
then
	CXXFLAGS="$CXXFLAGS -g"
fi


# Checks for wxwidgets libraries.
m4_include(wxwin.m4)
WX_CONFIG_OPTIONS
reqwx=3.1.4
WX_CONFIG_CHECK([$reqwx], [wxWin=1], [wxWin=0], [std,aui,propgrid], [])
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
		
fi

AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS_ONLY)
AC_SUBST(WX_CFLAGS_ONLY)
AC_SUBST(WX_LIBS)
AC_SUBST(WX_RESCOMP)

AC_SUBST(LIBS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)


AC_CONFIG_FILES([Makefile ])
AC_OUTPUT
