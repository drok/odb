# file      : odb/Makefile.am
# license   : GNU GPL v3; see accompanying LICENSE file

plugindir = @plugindir@

bin_PROGRAMS = odb
plugin_LTLIBRARIES = odb.la

AM_CPPFLAGS = -I'$(top_builddir)' -I'$(top_srcdir)'

# Note: not passed by libtool when linking odb.so. Seems to be harmless for
# now.
#
AM_CXXFLAGS = -std=c++0x

EXTRA_DIST = __path__(headers) __path__(extra_dist)

# Plugin.
#
odb_la_SOURCES = __path__(plugin_sources) __path__(common_sources)
odb_la_LDFLAGS = -module -shrext .so -avoid-version

# Remove the .la file from the final install.
#
install-data-hook:
	rm -f '$(DESTDIR)$(plugindir)/odb.la'

# Driver.
#
odb_SOURCES = __path__(driver_sources) __path__(common_sources)

# Make sure common sources are compiled differently.
#
odb_CXXFLAGS = $(AM_CXXFLAGS)
