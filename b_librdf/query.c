/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * example.c - Redland example parsing RDF from a URI, storing on disk as BDB hashes and querying the results
 *
 * Copyright (C) 2000-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2004, University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <redland.h>

#include "measures.h"

#define RDF_TYPE "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"
#define DBO_PERSON "http://dbpedia.org/ontology/Person"

int main_query(int argc, char *argv[]) 
{
  librdf_world* world;
  librdf_storage* storage;
  librdf_parser* parser;
  librdf_model* model;
  librdf_uri* base = NULL;
  librdf_stream* stream;
  librdf_node *predicate, *object;
  librdf_statement *partial_statement;
  char *program=argv[0];
  char *task=argv[1];
  char *filename=argv[2];
  char *parser_name=NULL;
  int count;
  raptor_world *raptor_world_ptr;

  unsigned long long t0, t1;
  double time_load, time_first = 0, time_rest;
  long m0, m1, mem_graph;

  fprintf(stderr, "task: %s\n", task);

  world = librdf_new_world();
  librdf_world_open(world);
  raptor_world_ptr = librdf_world_get_raptor(world);

  FILE* f = fopen((const char*)filename, "r");
  if(!f) {
    fprintf(stderr, "%s: Failed to open input file\n", program);
    return(1);
  }

  if (argc >= 4) {
    parser_name=argv[3];
  } else {
    parser_name="ntriples";
  }
  parser=librdf_new_parser(world, parser_name, NULL, NULL);
  if(!parser) {
    fprintf(stderr, "%s: Failed to create new parser\n", program);
    return(1);
  }

  if (argc >= 5) {
    base = librdf_new_uri(world, (const unsigned char*)argv[4]);
  } else {
    base = librdf_new_uri(world, (const unsigned char*)"http://example.org/");
  }

  m0 = get_vmsize();

  storage=librdf_new_storage(world, "memory", "test", NULL);
  if(!storage) {
    fprintf(stderr, "%s: Failed to create new storage\n", program);
    return(1);
  }

  model=librdf_new_model(world, storage, NULL);
  if(!model) {
    fprintf(stderr, "%s: Failed to create model\n", program);
    return(1);
  }
  
  /* PARSE the input file*/
  fprintf(stderr, "%s: Parsing %s\n", program, argv[1]);
  t0 = get_nanosec();
  if(librdf_parser_parse_file_handle_into_model(parser, f, 1, base, model)) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", program);
    return(1);
  } else {
    fprintf(stderr, "%s: parsed %d triples\n", program, librdf_model_size(model));
  }
  t1 = get_nanosec();
  time_load = (t1-t0)/1e9;

  m1 = get_vmsize();
  mem_graph = m1-m0;

  /* Construct the query predicate (arc) and object (target) 
   * and partial statement bits
   */
  predicate=librdf_new_node_from_uri_string(world, (const unsigned char*)RDF_TYPE);
  object=librdf_new_node_from_uri_string(world, (const unsigned char*)DBO_PERSON);
  if(!predicate || !object) {
    fprintf(stderr, "%s: Failed to create nodes for searching\n", program);
    return(1);
  }
  partial_statement=librdf_new_statement(world);
  librdf_statement_set_predicate(partial_statement, predicate);
  librdf_statement_set_object(partial_statement, object);

  /* QUERY TEST 1 - use find_statements to match */
  t0 = get_nanosec();
  stream=librdf_model_find_statements(model, partial_statement);
  if(!stream) {
    fprintf(stderr, "%s: librdf_model_find_statements returned NULL stream\n", program);
    return 1;
  }
  count=0;
  while(!librdf_stream_end(stream)) {
    librdf_statement *statement=librdf_stream_get_object(stream);
    if(!statement) {
      fprintf(stderr, "%s: librdf_stream_next returned NULL\n", program);
      return 1;
    }
    if (count == 0) {
      t1 = get_nanosec();
      time_first = (t1-t0)/1e9;
      t0 = get_nanosec();
    }
    //fputs("  Matched statement: ", stdout);
    //librdf_statement_print(statement, stdout);
    //fputc('\n', stdout);
    librdf_stream_next(stream);
    count++;
  }
  t1 = get_nanosec();
  time_rest = (t1-t0)/1e9;
  fprintf(stderr, "%s: got %d matching statements\n", program, count);


  printf("%f,%ld,%f,%f\n", time_load, mem_graph, time_first, time_rest);

  librdf_free_stream(stream);  
  librdf_free_uri(base);
  librdf_free_parser(parser);
  librdf_free_model(model);
  librdf_free_storage(storage);
  librdf_free_world(world);
#ifdef LIBRDF_MEMORY_DEBUG
  librdf_memory_report(stderr);
#endif
  return(0);
}

//// further code from the example file
//
//  /* Print out the model*/
//  fprintf(stdout, "%s: Resulting model is:\n", program);
//  raptor_iostream* iostr;
//  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, stdout);
//  librdf_model_write(model, iostr);
//  raptor_free_iostream(iostr);
//
//  //(...)
//
//
//  /* QUERY TEST 2 - use get_targets to do match */
//  librdf_iterator* iterator;
//  fprintf(stdout, "%s: Trying to get targets\n", program);
//  iterator=librdf_model_get_targets(model, subject, predicate);
//  if(!iterator)  {
//    fprintf(stderr, "%s: librdf_model_get_targets failed to return iterator for searching\n", program);
//    return(1);
//  }
//
//  count=0;
//  while(!librdf_iterator_end(iterator)) {
//    librdf_node *target;
//    
//    target=(librdf_node*)librdf_iterator_get_object(iterator);
//    if(!target) {
//      fprintf(stderr, "%s: librdf_iterator_get_object returned NULL\n", program);
//      break;
//    }
//
//    fputs("  Matched target: ", stdout);
//    librdf_node_print(target, stdout);
//    fputc('\n', stdout);
//
//    count++;
//    librdf_iterator_next(iterator);
//  }
//  librdf_free_iterator(iterator);
//  fprintf(stderr, "%s: got %d target nodes\n", program, count);
//
//  librdf_free_statement(partial_statement);
//  /* the above does this since they are still attached */
//  /* librdf_free_node(predicate); */
//  /* librdf_free_node(object); */
//
//}