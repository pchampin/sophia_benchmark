#!/usr/bin/env node

const { task,  filename, stream, get_vmsize, performance } = require("./common.js")
const { Stream } = require("stream");

global.Stream = Stream; // useful to make some types available in Rust

const sophia = require("../../sophia_wasm/sophia_wasm")

// listeners added in Rust do not trigger the stream,
// so add a dummy listener just to ensure it will be read
//stream.on('data', ()=>{});

const do_task = {
    'parse': parse_nt,
    'query': query_nt,
}[task];

do_task();


function parse_with_tester(stream, tester) {
    return new Promise((resolve, reject) => {
        stream.setEncoding("utf-8");
        stream.on('readable', () => {
            const txt = stream.read();
            if (txt) {
                tester.handle_data(txt);
            }
            else {
                tester.handle_end(txt);    
                resolve()
            }
        });
    });
}

function parse_nt() {
    const start = performance.now();
    const t = new sophia.make_tester_parse();
    parse_with_tester(stream, t)
    .then(() => {
        const duration = performance.now() - start;
        console.error("parsed: ", t.counter());
        console.log(duration/1000);
    })
    .catch((err) => {
        console.error(err);
    });
}

function query_nt() {
    let t_load, m_graph, t_first, t_rest;
    const mem0 = get_vmsize();
    let start = performance.now();
    let testerType = "make_tester_query";
    if (process.argv[4]) testerType += "_" + process.argv[4];
    console.error("tester: ", testerType);
    const t = new sophia[testerType]();
    parse_with_tester(stream, t)
    .then(() => {
        const duration = performance.now() - start;
        const mem1 = get_vmsize();
        console.error("parsed: ", t.counter());
        t_load = duration/1000;
        m_graph = mem1-mem0;
    })
    .then(() => {
        let counter = 0;
        start = performance.now();
        t.foreach_match(
            null,
            'http://www.w3.org/1999/02/22-rdf-syntax-ns#type',
            'http://dbpedia.org/ontology/Person',
            () => {
                if (counter == 0) {
                    duration = performance.now() - start;
                    t_first = duration/1000;
                    start = performance.now();
                }
                counter += 1;
            }
        )

        duration = performance.now() - start;
        t_rest = duration/1000;
        console.error(`retrieved: ${counter}`);
    })
    .then(() => {
        console.log(`${t_load},${m_graph},${t_first},${t_rest}`);
    })
    .catch((err) => {
        console.error(err);
    });
}

