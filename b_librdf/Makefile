.PHONY: test
test: all
	./run parse ../data/persondata_en_10k.ttl ntriples

.PHONY: all
all: run

run: run.c parse.o query.o measures.o
	clang run.c parse.o query.o measures.o -o run -Wall -lrdf -lraptor2

parse.o: parse.c measures.h
	clang -c parse.c -o parse.o -Wall -I/usr/include/raptor2 -I/usr/include/rasqal

query.o: query.c measures.h
	clang -c query.c -o query.o -Wall -I/usr/include/raptor2 -I/usr/include/rasqal

measures.o: measures.c measures.h
	clang -c measures.c -o measures.o -Wall

.PHONY: clean
clean:
	rm -f measures.o parse.o query.o run
