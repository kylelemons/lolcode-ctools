#include <iostream>
#include <cstdio>

#include <malloc.h>

#include "ast.h"

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
  ast_node *root = generate_ast();
  if (root != NULL)
    cout << "Valid A.S.T. generated." << endl;
  else
    cout << "No valid A.S.T. generated." << endl;

  return (root != NULL);
}
