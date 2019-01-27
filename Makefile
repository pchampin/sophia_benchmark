.PHONY: all
all:
	cd data; make
	cd b_sophia; make
	cd b_librdf; make
	cd b_jena; make
	cd b_python; make
	cd b_nodejs; make
