#!/usr/bin/env node

const n3 = require("n3");
const { task, filename, stream, get_vmsize, performance } = require("./common.js")

const do_task = {
    'parse': parse_nt,
    'query': query_nt,
}[task];

do_task();

function parse_nt() {
    let t_parse;
    const start = performance.now();
    const format = process.argv[4] || "N-Triples"; 
    let counter = 0;   
    let parser = new n3.Parser({ format: format });
    parser.parse(stream, (error, quad, prefixes) => {
        if (error) {
            console.error(error);
            process.exit(2);
        }
        if (quad) {
            counter += 1;
        } else {
            const duration = performance.now() - start;
            t_parse = duration/1000;
            console.error(`parsed: ${counter}`);
            console.log(`${t_parse}`);
            process.exit(0);
        };        
    });    
}

function query_nt() {
    let t_load, m_graph, t_first, t_rest;
    let start, duration;
    const format = process.argv[4] || "N-Triples";    
    let parser = new n3.Parser({ format: format });
    const mem0 = get_vmsize();
    start = performance.now();
    const store = new n3.Store();
    parser.parse(stream, (error, quad, prefixes) => {
        if (error) {
            console.error(error);
            process.exit(2);
        }
        if (quad) {
            store.addQuad(quad);
        } else {
            duration = performance.now() - start;
            const mem1 = get_vmsize();
            t_load = duration/1000;
            m_graph = mem1-mem0

            const predicate = n3.DataFactory.namedNode('http://www.w3.org/1999/02/22-rdf-syntax-ns#type');
            const object = n3.DataFactory.namedNode('http://dbpedia.org/ontology/Person');
            start = performance.now();
            let counter = 0;
            store.forEach((quad) => {
                if (counter == 0) {
                    duration = performance.now() - start;
                    t_first = duration/1000;
                    start = performance.now();
                }
                counter += 1;
            }, null, predicate, object);

            duration = performance.now() - start;
            t_rest = duration/1000;
            console.error(`retrieved: ${counter}`);
            console.log(`${t_load},${m_graph},${t_first},${t_rest}`);
            process.exit(0);
    };        
    });    
}

