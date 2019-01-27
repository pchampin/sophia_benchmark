#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raptor2.h>

#include "measures.h"

static char* program = NULL;

static int error_count = 0;
static int warning_count = 0;
static int ignore_warnings = 0;
static int ignore_errors = 0;

static void
rapper_log_handler(void *data, raptor_log_message *message)
{
  raptor_parser *parser = (raptor_parser *)data;
  
  switch(message->level) {
    case RAPTOR_LOG_LEVEL_FATAL:
    case RAPTOR_LOG_LEVEL_ERROR:
      if(!ignore_errors) {
        fprintf(stderr, "%s: Error - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
        
        raptor_parser_parse_abort(parser);
      }
      
      error_count++;
      break;

    case RAPTOR_LOG_LEVEL_WARN:
      if(!ignore_warnings) {
        fprintf(stderr, "%s: Warning - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
      }
      
      warning_count++;
      break;

    case RAPTOR_LOG_LEVEL_NONE:
    case RAPTOR_LOG_LEVEL_TRACE:
    case RAPTOR_LOG_LEVEL_DEBUG:
    case RAPTOR_LOG_LEVEL_INFO:

      fprintf(stderr, "%s: Unexpected %s message - ", program,
              raptor_log_level_get_label(message->level));
      raptor_locator_print(message->locator, stderr);
      fprintf(stderr, " - %s\n", message->text);
      break;
  }
  
}

static unsigned long nb_triples = 0;

static
void count_triple(void *user_data, raptor_statement *triple) 
{
    nb_triples += 1;
}

int main_parse(int argc, char** argv) {
  raptor_world* world = NULL;
  raptor_parser* rdf_parser = NULL;
  char *task = argv[1];
  char *filename = argv[2];
  char *format;
  unsigned char *uri_string;
  raptor_uri *input_uri = NULL, *base_uri = NULL;

  unsigned long long t0, t1;
  double time_parse;

  program = argv[0];
  fprintf(stderr, "task: %s\n", task);

  world = raptor_new_world();
  if(!world)
    exit(1);
  int rc = raptor_world_open(world);
  if(rc)
    exit(1);
  raptor_world_set_log_handler(world, rdf_parser, rapper_log_handler);

  uri_string = raptor_uri_filename_to_uri_string(filename);
  if(!uri_string) {
      fprintf(stderr, "%s: Failed to create URI for file %s.\n",
              program, filename);
      return(1);
  }
  input_uri = raptor_new_uri(world, uri_string);
  if(!input_uri) {
    fprintf(stderr, "%s: Failed to create URI for %s\n",
            program, uri_string);
    return(1);
  }

  if (argc >= 4) {
      format = argv[3];
  } else {
      format = "ntriples";
  }

  if (argc >= 5) {
      base_uri = raptor_new_uri(world, (unsigned char*) argv[4]);
      if(!base_uri) {
        fprintf(stderr, "%s: Failed to create URI for %s\n",
                program, argv[4]);
        return(1);
      }
  }

  rdf_parser = raptor_new_parser(world, format);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n",
            program, format);
    return(1);
  }
  raptor_parser_set_statement_handler(rdf_parser, rdf_parser, count_triple);

  t0 = get_nanosec();
  rc = raptor_parser_parse_file(rdf_parser, input_uri, base_uri);
  if (rc) {
      fprintf(stderr, "%s: Failed to parse file %s\n", program, filename);
      return 1;
  }
  t1 = get_nanosec();
  time_parse = (t1-t0)/1e9;
  printf("%f\n", time_parse);
  fprintf(stderr, "parsed: %ld triples\n", nb_triples);

  raptor_free_memory(uri_string);
  raptor_free_uri(input_uri);
  raptor_free_uri(base_uri);
  raptor_free_parser(rdf_parser);
  raptor_free_world(world);
  return 0;
}
