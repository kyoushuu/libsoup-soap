## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(LIBSOUP_SOAP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g


lib_LTLIBRARIES = libsoup-soap.la

libsoup_soap_la_SOURCES =

libsoup_soap_la_LDFLAGS =

libsoup_soap_la_LIBADD = $(LIBSOUP_SOAP_LIBS)


libsoup_soap_includedir = $(includedir)/libsoup-soap-0.1/libsoup-soap

libsoup_soap_include_HEADERS = \
	soup-soap.h


pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libsoup-soap-0.1.pc


EXTRA_DIST = \
	libsoup-soap-0.1.pc.in
