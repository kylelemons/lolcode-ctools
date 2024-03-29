#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// For constants
#include "grammar.tab.h"

typedef struct ast_node_t {
  unsigned char terminal; // boolean (1/0)
  unsigned type;
  unsigned nodecount;
  unsigned long lineno;
  void **nodes;
} ast_node;

extern const char * const * type_names;
extern unsigned type_count;

#ifdef __cplusplus
extern "C" {
#endif

ast_node *create_ast_node(unsigned t, unsigned l, unsigned char term);

void append_leaf(ast_node *node, void *leaf);

void print_tree(ast_node *node, unsigned indent);

// NOTE! THIS IS NOT IN AST.C
ast_node *generate_ast();

#ifdef __cplusplus
}
#endif

// defines to make this all shorter
#define CN(x,l) create_ast_node(x,l,0)
#define CT(x,l) create_ast_node(x,l,1);
#define AL(x,y) append_leaf(x,y)
#define ALL(x,y,z) AL(x,y),AL(x,z)
#define ALLL(x,y,z,w) ALL(x,y,z),AL(x,w)

// Print white space
#define ws(c) { int _local_i; for (_local_i = 0; _local_i < c; ++_local_i) printf(" "); }

#endif
