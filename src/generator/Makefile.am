AM_CPPFLAGS=-I$(top_srcdir)/include
EXTRA_DIST=testite.sh slider3_base.py
noinst_LIBRARIES = libgen.a
libgen_a_SOURCES= xorgen.cc vanDerWaerden.cc rn.cc slider2.cc trans.cc \
rksat.cc add_tree.cc #rand_bdd.cc

bin_PROGRAMS = gentest
gentest_SOURCES = gentest.cc
gentest_LDADD = libgen.a #../libsbsat.la $(LIBM)

bin_SCRIPTS = slider3.py 
CLEANFILES = $(bin_SCRIPTS) gentest-seded.cc

slider3.py: slider3_base.py
	rm -f $(top_builddir)/src/generator/slider3.py
	echo "#! " $(PYTHON) > $(top_builddir)/src/generator/slider3.py
	cat $(top_srcdir)/src/generator/slider3_base.py >> $(top_builddir)/src/generator/slider3.py
	chmod +x $(top_builddir)/src/generator/slider3.py
