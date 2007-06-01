#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "ast.h"

int main(int argc, char **argv)
{
  ast_node *root = generate_ast();
  if (root != NULL)
    printf ("Valid A.S.T. Generated\n");
  else
    printf ("No A.S.T. Generated.\n");

  return 0;
}
