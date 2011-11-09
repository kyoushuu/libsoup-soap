dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT([LibSoup-SOAP], [0.1],
		[https://sourceforge.net/p/libsoup-soap/tickets/],
		[libsoup-soap],
		[http://libsoup-soap.sourceforge.net/])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror gnu no-dist-gzip dist-xz])

AM_SILENT_RULES([yes])

AC_PROG_CC




dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=libsoup-soap
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT


LT_INIT


PKG_CHECK_MODULES(LIBSOUP_SOAP, [libxml-2.0 libsoup-2.4 glib-2.0 gobject-2.0 ])




AC_OUTPUT([
Makefile
libsoup-soap/libsoup-soap-0.1.pc
libsoup-soap/Makefile
po/Makefile.in])
