#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

// For constants
#include "grammar.tab.h"
#include "ast.h"

const char * const * type_names = NULL;
unsigned type_count = 0;

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

void print_tree(ast_node *node, unsigned indent)
{
  if (!node)
  {
    ws(indent); printf("<NULL>\n");
    return;
  }
  if (node->type >= type_count)
  {
    ws(indent); printf("Huh? TN = %d\n", node->type);
  }
  ws(indent); printf("{%s:%d:%p}\n", type_names[node->type], node->nodecount, node->nodes);
  if (node->terminal == 0)
  {
    unsigned n;
    for (n = 0; n < node->nodecount; ++n)
    {
      if (node->nodes[n])
      {
        print_tree(node->nodes[n], indent+2);
      }
      else
      {
        ws(indent+2); printf("Uh ho! NULL\n");

      }
    }
  }
  //ws(indent+2),printf("<terminal>\n");
}
