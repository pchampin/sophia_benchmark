#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "measures.h"

unsigned long long get_nanosec() {
  int status;
  struct timespec timespec;
  unsigned long long ret = 0;
  status = clock_gettime(CLOCK_MONOTONIC, &timespec);
  if (status != 0) {
    perror("error in clock_gettime:");
    exit(1);
  }
  ret += (unsigned long long) timespec.tv_sec * 1000000000;
  ret += (unsigned long long) timespec.tv_nsec;
  return ret;
}

int parse_line(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int get_vmsize(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parse_line(line);
            break;
        }
    }
    fclose(file);
    return result;
}