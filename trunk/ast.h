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

extern char **type_names;

ast_node *create_ast_node(unsigned t, unsigned char term);

void append_leaf(ast_node *node, void *leaf);

// defines to make this all shorter
#define CN(x) create_ast_node(x,0)
#define CT(x) create_ast_node(x,1);
#define AL(x,y) append_leaf(x,y)
#define ALL(x,y,z) AL(x,y),AL(x,z)
#define ALLL(x,y,z,w) ALL(x,y,z),AL(x,w)
