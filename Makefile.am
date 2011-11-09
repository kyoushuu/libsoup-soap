## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = libsoup-soap po

libsoup_soapdocdir = ${prefix}/doc/libsoup-soap
libsoup_soapdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = $(libsoup_soapdoc_DATA) \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(libsoup_soapdocdir)
