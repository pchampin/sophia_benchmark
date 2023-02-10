JENA=apache-jena-4.7.0

.PHONY: all
all: build test

.PHONY: build
build: JenaBenchmark.class

${JENA}:
	wget https://www-eu.apache.org/dist/jena/binaries/${JENA}.tar.gz
	tar xzf ${JENA}.tar.gz
	rm ${JENA}.tar.gz
	sed -i 's|INFO|OFF|' "${JENA}/log4j2.properties"

JenaBenchmark.class: ${JENA} JenaBenchmark.java
	javac -cp "`./cp.sh`" JenaBenchmark.java

.PHONY: clean
clean:
	rm -f JenaBenchmark.class

.PHONY: clean-all
clean-all: clean
	rm -rf ${JENA}

.PHONY: test
test: JenaBenchmark.class
	./run query ../data/persondata_en_10k.ttl
