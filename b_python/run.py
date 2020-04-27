#!/usr/bin/env python

from sys import argv, stderr
import re
from time import clock_getres, clock_gettime, CLOCK_MONOTONIC

from rdflib import Graph, RDF, URIRef

assert clock_getres(CLOCK_MONOTONIC) == 1e-9
VMSIZE_RE = re.compile(r"VmSize\s*:\s*([0-9]*)")

def get_vmsize():
    with open("/proc/self/status") as f:
        txt = f.read()
    return int(VMSIZE_RE.search(txt).group(1))

def get_time():
    return clock_gettime(CLOCK_MONOTONIC)

def task_parse():
    raise NotImplementedError()

def task_query(query_num=1):
    filename = argv[2]
    syntax = argv[3] if len(argv) > 3 else "nt"
    m0 = get_vmsize()
    t0 = get_time()
    g = Graph()
    g.parse(filename, format=syntax)
    t1 = get_time()
    m1 = get_vmsize()
    time_load = t1-t0
    mem_graph = m1-m0

    patterns = {
        1: (None, RDF.type, URIRef("http://dbpedia.org/ontology/Person")),
        2: (URIRef("http://dbpedia.org/resource/Vincent_Descombes_Sevoie"), None, None),
    }
    pattern = patterns[query_num];
    time_first = None
    c = 1
    t0 = get_time()
    for triple in g.triples(pattern):
        if time_first is None:
            time_first = get_time() - t0
            t0 = get_time()
        c += 1
    time_rest = get_time()-t0
    print("matching triples: {}".format(c), file=stderr)

    print("{},{},{},{}".format(
        time_load, mem_graph, time_first, time_rest
    ))



def main():
    if argv[1] == 'parse':
        task_parse()
    elif argv[1] == 'query':
        task_query()
    elif argv[1] == 'query2':
        task_query(2)
    


if __name__ == '__main__':
    main()