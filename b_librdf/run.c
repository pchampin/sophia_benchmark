#include <stdio.h>
#include <string.h>

int main_parse(int argc, char *argv[]);
int main_query(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    if (argc <3) {
        fprintf(stderr, "Usage: %s <task> <filename> [options...]\n", argv[0]);
        return 1;
    }
    const char* task = argv[1];
    if (strcmp(task, "parse") == 0) {
        return main_parse(argc, argv);
    } else if (strcmp(task, "query") == 0) {
        return main_query(argc, argv);
    } else {
        fprintf(stderr, "Unkonwn task %s\n", task);
        return 1;
    }
}
