use std::{env, fs, io, process};
use std::io::Write;
use std::str::FromStr;

extern crate regex;
use regex::Regex;

extern crate time;
use time::OffsetDateTime;

extern crate sophia;
use sophia::graph::*;
use sophia::graph::inmem::*;
use sophia::ns::rdf;
use sophia::parser::nt;
use sophia::triple::stream::*;
use sophia::term::Term;

fn get_vmsize() -> usize {
    let status = fs::read_to_string("/proc/self/status").unwrap();
    let vmsize_re = Regex::new(r"VmSize:\s*([0-9]+) kB").unwrap();
    let vmsize = vmsize_re.captures(&status).unwrap().get(1).unwrap().as_str();
    usize::from_str(vmsize).unwrap()
}

fn task_query<R> (f: R, variant: Option<&str>) where
    R: io::BufRead,
{
    eprintln!("task    : query");
    match variant {
        None => { 
            task_query_g(f, FastGraph::new());
        }
        Some("light") => {
            task_query_g(f, LightGraph::new());
        }
        Some(v) => {
            eprintln!("Unknown variant {}", v);
            process::exit(1);
        }
    };
}

fn task_query_g<G, R> (f: R, mut g: G) where
    R: io::BufRead,
    G: MutableGraph,
{
    let m0 = get_vmsize();
    let t0 = OffsetDateTime::now();
    g.insert_all(nt::parse_bufread(f)).expect("Error parsing NT file");
    let t1 = OffsetDateTime::now();
    let m1 = get_vmsize();
    let time_parse = (t1-t0).as_seconds_f64();
    let mem_graph = m1-m0;
    eprintln!("loaded  : ~ {:?} triples\n", g.triples().size_hint());

    let mut time_first: f64 = 0.0;
    let time_rest;
    let dbo_person = Term::<&'static str>::new_iri("http://dbpedia.org/ontology/Person").unwrap();

    let mut t0 = OffsetDateTime::now();
    let results = g.triples_with_po(&rdf::type_, &dbo_person);
    let mut c = 0;
    for _ in results {
        if c == 0 {
            let t1 = OffsetDateTime::now();
            time_first = (t1-t0).as_seconds_f64();
            t0 = OffsetDateTime::now();
        }
        c += 1;
    }
    let t1 = OffsetDateTime::now();
    time_rest = (t1-t0).as_seconds_f64();
    eprintln!("matching triple: {}\n", c);

    println!("{},{},{},{}", time_parse, mem_graph, time_first, time_rest);
}

fn task_parse<T: io::BufRead> (f: T, variant: Option<&str>) {
    eprintln!("task    : parse");
    match variant {
        None => {
            task_parse_nt(f);
        }
        Some("nt") => {
            task_parse_nt(f);
        }
        Some(v) => {
            eprintln!("Unknown variant {}", v);
            process::exit(1);
        }
    };
}

fn task_parse_nt<T: io::BufRead> (f: T) {
    let t0 = OffsetDateTime::now();
    nt::parse_bufread(f).for_each_triple(|_| ()).expect("Error parsing NT file");
    let t1 = OffsetDateTime::now();
    let time_parse = (t1-t0).as_seconds_f64();
    println!("{}", time_parse);
}


fn main() {
    eprintln!("program : sophia");
    eprintln!("pid     : {}", process::id());
    let args: Vec<String> = env::args().collect();
    if args.len() < 3 {
        io::stderr().write(b"usage: sophia_benchmark <task> <filename.nt>\n").unwrap();
        process::exit(1);
    }
    let task_id: &str = &args[1];
    let filename = &args[2];
    let variant = if args.len() > 3 {
        Some(&args[3] as &str)
    } else {
        None
    };
    eprintln!("filename: {}", filename);
    let f = fs::File::open(&filename).expect("Error opening file");
    let f = io::BufReader::new(f);
    match task_id {
        "parse"  => task_parse(f, variant),
        "query" => task_query(f, variant),
        _   => {
            eprint!("Unknown task {}", task_id);
            process::exit(1);
        }
    };
}
