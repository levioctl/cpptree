AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LTLIBRARIES = libtreeprinter.la libpaginator.la

libtreeprinter_la_SOURCES = treeprinter.cpp paginator.cpp
libtreeprinter_la_LIBADD = -lncursesw

libpaginator_la_SOURCES = paginator.cpp
