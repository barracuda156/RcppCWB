extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cl.h>
#include <pcre.h>
#include "globals.h"
#include "context_descriptor.h"
#include "_options.h"
#include "cqp.h"
#include "corpmanag.h"
#include "server.h"
}

#include <Rcpp.h>
#include <stdio.h>
#include <string.h>


using namespace Rcpp;




// [[Rcpp::export(name=".init_cqp")]]
void init_cqp() {
	int		ac = 1;
	char *		av[1];
	av[0] = (char *)"RcppCWB";
	which_app = cqp;
	silent = 1; 
	paging = 0;
	autoshow = 0;
	auto_save = 0;
	server_log = 0;
	enable_macros = 0;

	initialize_cqp(ac, av);
	make_attribute_hash(16384);
}


// [[Rcpp::export(name=".get_cqp_registry")]]
Rcpp::StringVector get_cqp_registry(){
  Rcpp::StringVector result(1);
  result(0) = cl_standard_registry();
  return result;
}


// [[Rcpp::export(name=".set_cqp_registry")]]
SEXP set_cqp_registry(SEXP registry_dir){
  char * registry_new;
  registry_new = (char*)CHAR(STRING_ELT(registry_dir,0));
  SEXP result = R_NilValue;
  setenv("CORPUS_REGISTRY", registry_new, 1);		
  init_cqp();
  return result;
}


// [[Rcpp::export(name=".cwb_list_corpora")]]
Rcpp::StringVector cwb_list_corpora(){
  
  CorpusList *	cl;
  int	i = 0, n = 0;
  
  /* First count corpora */
  for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
    if (cl->type == SYSTEM) n++;
  }
  Rcpp::StringVector result(n);

  /* Then build list of names */
  for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
    if (cl->type == SYSTEM) {
      result(i) = cl->name;
      i++;
    }
  }
  return result;
  
}


// [[Rcpp::export(name=".cqp_query")]]
SEXP cqp_query(SEXP corpus, SEXP subcorpus, SEXP query){
  
  char * mother = (char*)CHAR(STRING_ELT(corpus,0));
  char * child = (char*)CHAR(STRING_ELT(subcorpus,0));
  char * q = (char*)CHAR(STRING_ELT(query,0));
  char * cqp_query;
  CorpusList *cl;
  
  
  cl = cqi_find_corpus(mother);
  set_current_corpus(cl, 0);

  int len = strlen(child) + strlen(q) + 10;
  
  cqp_query = (char *) cl_malloc(len);
  sprintf(cqp_query, "%s = %s", child, q);
  
  if (!cqi_activate_corpus(mother)){
    printf("activation failed");
  }
  if (!check_subcorpus_name(child)){
    printf("checking subcorpus name failed \n");
  }
  
  cqp_parse_string(cqp_query);

  char *			full_child;
  CorpusList *	childcl;
  
  full_child = combine_subcorpus_spec(mother, child); /* c is the 'physical' part of the mother corpus */
  childcl = cqi_find_corpus(full_child);
  /* if ((childcl) == NULL) {
    printf("subcorpus not found\n");
  } */
  
  SEXP result = R_NilValue;
  return result;
}


// [[Rcpp::export(name=".cqp_subcorpus_size")]]
int cqp_subcorpus_size(SEXP scorpus)
{
  int result;
  char * subcorpus;
  CorpusList * cl;
  
  subcorpus = (char*)CHAR(STRING_ELT(scorpus,0));
  cl = cqi_find_corpus(subcorpus);
  
  if (cl == NULL) {
    result = 0;
  } else {
    result = cl->size;
  }
  return result;
}

// [[Rcpp::export(name=".cqp_list_subcorpora")]]
Rcpp::StringVector cqp_list_subcorpora(SEXP inCorpus)
{
  char * corpus;
  CorpusList *cl, *mother;
  int i = 0, n = 0;
  Rcpp::StringVector result;
  
  corpus = (char*)CHAR(STRING_ELT(inCorpus,0));
  
  mother = cqi_find_corpus(corpus);
  if (!check_corpus_name(corpus) || mother == NULL) {
    Rcpp::StringVector result(1);
  } else {
    /* First count subcorpora */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus) n++;
    }
    
    Rcpp::StringVector result(n);

    /* Then build list of names */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus) {
        result(i) = cl->name;
        i++;
      }
    }
  }
  return result;
}

// [[Rcpp::export(name=".cqp_dump_subcorpus")]]
Rcpp::IntegerMatrix cqp_dump_subcorpus(SEXP inSubcorpus)
{
  char * subcorpus;
  CorpusList * cl;
  int i;
  int nrows = cqp_subcorpus_size(inSubcorpus);

  subcorpus = (char*)CHAR(STRING_ELT(inSubcorpus,0));
  cl = cqi_find_corpus(subcorpus);
  if (cl == NULL) {
    printf("subcorpus not found\n");
  }
  
  Rcpp::IntegerMatrix result(nrows,2);

  for (i = 0; i < nrows; i++) {
    result(i,0) = cl->range[i].start;
    result(i,1) = cl->range[i].end;
  }
  
  return result;
}


