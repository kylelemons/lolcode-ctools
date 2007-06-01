#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// For constants
#include "grammar.tab.h"
#include "ast.h"

char **type_names;

ast_node *create_ast_node(unsigned t, unsigned char term)
{
  //printf("\n[[Creating %snode:%d:", (term?"terminal ":""), t);
  //fflush(stdout);
  //printf("%s]]\n", (type_names?type_names[t]:""));
  ast_node *node = (ast_node*)calloc(1, sizeof(ast_node));
  node->type = t;
  node->terminal = term;
  return node;
}

void append_leaf(ast_node *i_node, void *leaf)
{
  ast_node *node = (ast_node*)i_node;
  node->nodes = realloc(node->nodes, (node->nodecount + 1) * sizeof(void*));
  node->nodes[node->nodecount++] = leaf;
}