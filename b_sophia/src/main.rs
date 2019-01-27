use std::{env, fs, io, process};
use std::borrow::Borrow;
use std::collections::HashSet;
use std::io::Write;
use std::mem::size_of;
use std::rc::Rc;
use std::str::FromStr;

extern crate regex;
use regex::Regex;

extern crate time;
use time::precise_time_ns;

extern crate sophia;
use sophia::graph::*;
use sophia::graph::inmem::*;
use sophia::ns::rdf;
use sophia::parsers::nt;
use sophia::streams::*;
use sophia::term::{Normalization, RcTerm, Term, factory::*};
use sophia::triple::Triple;

fn get_vmsize() -> usize {
    let status = fs::read_to_string("/proc/self/status").unwrap();
    let vmsize_re = Regex::new(r"VmSize:\s*([0-9]+) kB").unwrap();
    let vmsize = vmsize_re.captures(&status).unwrap().get(1).unwrap().as_str();
    usize::from_str(vmsize).unwrap()
}

fn task_0<T: io::BufRead> (f: T) {
    eprintln!("task    : wc (0)");
    let mut c = 0;
    for _line in f.lines() {
        c += 1;
    }
    eprintln!("counted : {}", c);
}

fn task_1<T: io::BufRead> (f: T) {
    eprintln!("task    : parse (1)");
    let t0 = precise_time_ns();
    nt::parse_read(f).into_sink(&mut ()).expect("Error parsing NT file");
    let t1 = precise_time_ns();
    let time_parse = (t1-t0) as f64/1e9;
    println!("{}", time_parse);
}

fn task_2<T: io::BufRead> (f: T) {
    eprintln!("task    : load_nt_into_hashset (2)");
    let mut g = HashSet::new();
    nt::parse_read(f).into_graph(&mut g).expect("Error parsing NT file");
}

fn task_3<T: io::BufRead> (f: T) {
    eprintln!("task    : load_nt_into_very_simple_graph (3)");
    let mut g = VerySimpleGraph::new();
    nt::parse_read(f).into_graph(&mut g).expect("Error parsing NT file");

    let mut txt_len = 0;
    for i in g.factory.iter() {
        if Rc::strong_count(&i) > 2 {
            //eprintln!("{} -------- {:?}", Rc::strong_count(&i), i.as_ref())
            txt_len += i.len() + size_of::<Rc<str>>();
        }
    }
    eprintln!("factory_used    : {}", txt_len);
    eprintln!("factory_occupied: {}", txt_len + (g.factory.capacity()-g.factory.len())*size_of::<Rc<str>>());

    let triple_size = size_of::<(RcTerm, RcTerm, RcTerm)>();
    eprintln!("vec_used    : {}", g.triples.len() * triple_size);
    eprintln!("vec_occupied: {}", g.triples.capacity() * triple_size);
}

fn task_query<F, G, R> (f: R, factory: F) where
    R: io::BufRead,
    G: MutableGraph,
    F: FnOnce() -> G,
{
    let m0 = get_vmsize();
    let t0 = precise_time_ns();
    let mut g = factory();
    nt::parse_read(f).into_graph(&mut g).expect("Error parsing NT file");
    let t1 = precise_time_ns();
    let m1 = get_vmsize();
    let time_parse = (t1-t0) as f64/1e9;
    let mem_graph = m1-m0;
    eprintln!("loaded  : ~ {:?} triples\n", g.hint());

    let mut time_first: f64 = 0.0;
    let time_rest;
    let dbo_person = Term::<&'static str>::new_iri("http://dbpedia.org/ontology/Person").unwrap();

    let mut t0 = precise_time_ns();
    let results = g.iter_for_po(&rdf::type_, &dbo_person);
    let mut c = 0;
    for _ in results {
        if c == 0 {
            let t1 = precise_time_ns();
            time_first = (t1-t0) as f64/1e9;
            t0 = precise_time_ns();
        }
        c += 1;
    }
    let t1 = precise_time_ns();
    time_rest = (t1-t0) as f64/1e9;
    eprintln!("matching triple: {}\n", c);

    println!("{},{},{},{}", time_parse, mem_graph, time_first, time_rest);
}

fn task_4<T: io::BufRead> (f: T) {
    eprintln!("task    : query w/ fast_graph (4)");
    task_query(f, &FastGraph::new);
}

fn task_5<T: io::BufRead> (f: T) {
    eprintln!("task    : query w/ light_graph (5)");
    task_query(f, &LightGraph::new);
}

fn task_6(filename: &str) {
    eprintln!("task    : parse-alt (6)");
    let content = fs::read_to_string(filename).unwrap();
    let t0 = precise_time_ns();
    nt::parse_str(&content).into_sink(&mut ()).expect("Error parsing NT file");
    let t1 = precise_time_ns();
    let time_parse = (t1-t0) as f64/1e9;
    println!("{}", time_parse);
}

fn task_7<T: io::BufRead> (_f: T) {
    unimplemented!()
    /*
    eprintln!("task    : parse + count (7)");
    let t0 = precise_time_ns();
    nt::count_read(f).expect("Error parsing NT file");
    let t1 = precise_time_ns();
    let time_parse = (t1-t0) as f64/1e9;
    println!("{}", time_parse);
    */
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
    eprintln!("filename: {}", filename);
    let f = fs::File::open(&filename).expect("Error opening file");
    let f = io::BufReader::new(f);
    match task_id {
        "parse"  => task_1(f),
        "query" => task_4(f),
        "0" => task_0(f),
        "1" => task_1(f),
        "2" => task_2(f),
        "3" => task_3(f),
        "4" => task_4(f),
        "5" => task_5(f),
        "6" => task_6(&filename),
        "7" => task_7(f),
        _   => task_0(f),
    };
}


#[derive(Clone)]
struct DummyStr {}

impl Borrow<str> for DummyStr {
    fn borrow(&self) -> &str {
        ""
    }
}

impl<'a> From<&'a str> for DummyStr {
    fn from(_: &'a str) -> DummyStr {
        DummyStr{}
    }
}



struct VerySimpleGraph {
    triples: Vec<(RcTerm, RcTerm, RcTerm)>,
    factory: RcTermFactory,
}

impl VerySimpleGraph {
    fn new() -> VerySimpleGraph {
        VerySimpleGraph {
            triples: Vec::new(),
            factory: RcTermFactory::new(),
        }
    }

    fn copy<T: Borrow<str>> (&mut self, t: &Term<T>) -> RcTerm {
        self.factory.copy_normalized(t, Normalization::LastHashOrSlash)
    }
}

impl Graph for VerySimpleGraph {
    type Holder = Rc<str>;
    type Error = sophia::error::Never;

    fn iter(&self) -> GFallibleTripleIterator<Self> {
        self.triples.iter()
    }
}

impl MutableGraph for VerySimpleGraph {

    fn insert<T, U, V> (&mut self, s: &Term<T>, p: &Term<U>, o: &Term<V>) -> Result<bool, Self::Error> where
        T: Borrow<str>,
        U: Borrow<str>,
        V: Borrow<str>,
    {
        let s = self.copy(s);
        let p = self.copy(p);
        let o = self.copy(o);

        let cap = self.triples.capacity();
        if cap == self.triples.len() {
            if cap < 1024 {
                self.triples.reserve(cap);
            } else {
                self.triples.reserve(cap/4);
            }
            eprintln!("=== new cap {}", self.triples.capacity());
        }
        self.triples.push((s, p, o));
        Ok(true)
    }

    fn remove<T, U, V> (&mut self, s: &Term<T>, p: &Term<U>, o: &Term<V>) -> Result<bool, Self::Error> where
        T: Borrow<str>,
        U: Borrow<str>,
        V: Borrow<str>,
    {
        let mut to_remove = None;
        for (i, t) in self.triples.iter().map(|res| res.unwrap()).enumerate() {
            if t.s() == s && t.p() == p && t.o() == o {
                to_remove = Some(i);
                break
            }
        }
        if let Some(i) = to_remove {
            self.triples.remove(i);
            Ok(true)
        } else {
            Ok(false)
        }
    }
}


