#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// For constants
#include "grammar.tab.h"

typedef struct ast_node_t {
  unsigned char terminal; // boolean (1/0)
  unsigned type;
  unsigned nodecount;
  void **nodes;
} ast_node;

extern unsigned type_count;
extern const char * const * type_names;

ast_node *create_ast_node(unsigned t, unsigned char term);

void append_leaf(ast_node *node, void *leaf);

void print_tree(ast_node *node, unsigned indent);

// NOTE! THIS IS NOT IN AST.C
ast_node *generate_ast();

// defines to make this all shorter
#define CN(x) create_ast_node(x,0)
#define CT(x) create_ast_node(x,1);
#define AL(x,y) append_leaf(x,y)
#define ALL(x,y,z) AL(x,y),AL(x,z)
#define ALLL(x,y,z,w) ALL(x,y,z),AL(x,w)

// Print white space
#define ws(c) { int _local_i; for (_local_i = 0; _local_i < c; ++_local_i) printf(" "); }
