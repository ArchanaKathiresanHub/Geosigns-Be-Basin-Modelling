AUTOMAKE_OPTIONS = foreign 1.5
ACLOCAL_AMFLAGS = -I m4
SUBDIRS=src

include $(top_srcdir)/aminclude.am
EXTRA_DIST = autogen.sh $(DX_CONFIG) doxygen-doc/html doxygen-doc/latex doc/ m4/

clean-doxy:
	rm -rf doxygen-doc/ 