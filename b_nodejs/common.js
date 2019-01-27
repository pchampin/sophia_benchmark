const fs = require("fs");
const { performance } = require("perf_hooks");

TASKS = ['parse', 'query'];

const filename = process.argv[3];
if (filename === undefined) {
    console.error(`usage: node ${process.argv[1]} <task> <filename>}`);
    process.exit(1);
}

const task = process.argv[2];
if (TASKS.indexOf(task) === -1) {
    console.error(`task must be one of ${TASKS}`);
    process.exit(1);
}
console.error(`task: ${task}`);
const stream = fs.createReadStream(filename);

function get_vmsize() {
    txt = fs.readFileSync("/proc/self/status", encoding="utf8");
    val = txt.match(/VmSize:\W*([0-9]+) kB/).pop();
    return Number.parseInt(val);
}

exports.performance = performance;
exports.filename = filename;
exports.task = task;
exports.stream = stream;
exports.get_vmsize = get_vmsize;
