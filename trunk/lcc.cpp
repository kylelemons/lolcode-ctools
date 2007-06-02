#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

#include <string>
using std::string;

#include "lolcode.hpp"
using namespace LOLCode;

#include <cstdio>
#include <malloc.h>
#include <getopt.h>

#include "ast.h"


/*!
 * Print usage statement
 */

void usage(const char *progname)
{
  cerr << "Usage: " << progname << " [-Cvcpo]" << endl;
  cerr << "  -v           Verbose output" << endl;
  cerr << "  -C           Check only (enable verbose output and disable compiling)" << endl;
  cerr << "  -c           Compile into Assembly (default)" << endl;
  cerr << "  -p           Print out the nodes in the A.S.T. (advanced)" << endl;
  cerr << "  -o <file>    Write compiler output to <file> (default: out.s)" << endl;
  exit(1);
}

/*!
 * Program execution entry point
 */

int main(int argc, char **argv)
{
  static char *options = "Cvcpo:";

  bool verbose = false;
  bool compile = true;
  bool print_ast = false;

  string output_file = "out.s";

  // option parsing
  while (true)
  {
    static char c;
    opterr = 0;
    c = getopt(argc, argv, options);
    if (c == -1) break;
    switch (c)
    {
      case 'p':
        print_ast = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'C':
        verbose = true;
        compile = false;
        break;
      case 'c':
        compile = true;
        break;
      case 'o':
        output_file = string(optarg);
        break;
      default:
        cerr << "Unrecognized option: -" << (char)((c=='?')?optopt:c) << endl;
        usage(*argv);
        break;
    }
  }

  ASTNode *root = generate_ast();

  if (root == NULL)
  {
    if (verbose)
      cout << "No valid A.S.T. generated." << endl;
    return 1;
  }
  if (verbose)
    cout << "Valid A.S.T. generated!" << endl;
  if (print_ast)
    print_tree(root, 0);
  
  CompilerContext context;
  try
  {
    if (compile)
    {
      HookFunc run = hook_search( type_names[root->type] );
      run(root, context);
    }
  }
  catch (HookError e)
  {
    cout << "Error in compiling:" << endl;
    cout << "  " << e.to_string() << endl;
    cout << "Call stack:" << endl;
    cout << e.backtrace() << flush; // appends newline for us
    return 1;
  }

  return (root == NULL);
}
