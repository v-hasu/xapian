bin_PROGRAMS +=\
	bin/questletor\
	bin/xapian-letor-update\
	bin/letor_evaluation

bin_questletor_SOURCES =\
	bin/questletor.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_questletor_LDADD = libxapianletor.la

bin_xapian_letor_update_SOURCES =\
	bin/xapian-letor-update.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_xapian_letor_update_LDADD = $(XAPIAN_LIBS)

bin_letor_evaluation_SOURCES =\
	bin/letor_evaluation.cc\
	common/getopt.cc\
	common/gnu_getopt.h
bin_letor_evaluation_LDADD = libxapianletor.la

if !MAINTAINER_NO_DOCS
dist_man_MANS +=\
	bin/questletor\
	bin/xapian-letor-update\
	bin/letor_evaluation
endif

if DOCUMENTATION_RULES
bin/questletor: bin/questletor$(EXEEXT) makemanpage
	./makemanpage bin/questletor $(srcdir)/bin/questletor.cc bin/questletor

bin/xapian-letor-update: bin/xapian-letor-update$(EXEEXT) makemanpage
	./makemanpage bin/xapian-letor-update $(srcdir)/bin/xapian-letor-update.cc bin/xapian-letor-update

bin/letor_evaluation: bin/letor_evaluation$(EXEEXT) makemanpage
	./makemanpage bin/letor_evaluation $(srcdir)/bin/letor_evaluation.cc bin/letor_evaluation

endif
