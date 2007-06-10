#ifndef LOLCODE_H
#define LOLCODE_H

#include <string>
#include <vector>
#include <stack>
#include <map>

#include "ast.h"

/*!
 * \brief LOLCode parser/compiler/interpreter data/functions
 *
 *   This contains all of the functions, data, etc that the LOLCode
 * parser/compiler/interpreter/whatevers need to run
 */

namespace LOLCode
{
  using std::string;
  using std::vector;
  using std::stack;
  using std::map;

  /*!
   * \brief Holds the current execution context of the compiler
   *
   * This holds all of the information that the compiler needs to keep track of
   * throughout its execution, and it is passed by reference throughout the execution
   * of the Abatract Syntax Tree.  This contains things like the final output, the headers
   * that need to be set before the final output, the current state of various context-specific
   * compile information, etc.
   */

  class CompilerContext
  {
    public:
      const static int tab_width = 20;
      const static string ret_reg;
      const static string var_reg;
      const static string cnt_reg;
      const static string val_reg;
      const static string dim_reg;
      const static string ptr_reg;
      const static string frame_ptr;
      const static string stack_ptr;

      unsigned int counter; /*!< This counter remains unique and should only increment */
      string filename; /*!< Set this to the input filename (used in comment generation) */

      map<string,bool> flags; /*!< Holds various flags that should persist */
      vector<string> header_pieces; /*!< Holds the source of the output program's header */
      vector<string> file_pieces; /*!< Holds the source of the output program */
      map<string, map<string,int> > offset; /*!< Holds the symbols we're using and their offsets */
      map<string, map<string,string> > variables; /*!< Holds the variables that we're using and their types */
      map<string, map<string,vector<int> > > dimensions; /*!< Holds the sizes of the arrays we're using */

      map<int,string> int_constants; /*!< Holds the integer constants and their names */
      map<string,string> string_constants; /*!< Holds the string constants and their names */

      stack<int> int_stack; /*!< Stack up ints for passing back and forth between functions */
      stack<string> string_stack; /*!< Stack up strings for passing back and forth between functions */ 
      stack<string> context_stack; /*!< Stack up strings representing what blocks we're in */
      stack<string> varcontext_stack; /*!< Stack up strings when we change variable exclusive scope */

      map<string,int> mem_stack; /*!< Holds where the current %esp is relative to %ebp (for use in allocating local variables) */

      CompilerContext();

      void header(string piece, unsigned line = 0);
      void header_raw(string piece);
      void output(string piece, string comment = "");
      void output(string piece, unsigned lineno);
      void output_raw(string piece);
      string build_file(); 
  };

  /*! \brief The Abstract Syntax Tree (A.S.T) Node */
  typedef ast_node ASTNode;
  /*! \brief The types of functions stored in hooks */
  typedef void (*HookFunc)(ASTNode *node, CompilerContext &context);

  /*!
   * \brief A function used to parse an A.S.T.
   */
  typedef struct {
    const char *id;               /*!< The name of the rule that made the node to parse */
    HookFunc func;               /*!< the address of the function to call */
  } Hook;

  /*! A list of hooks for the A.S.T. nodes */
  extern const Hook hooks[];

  /*!
   * \brief Thrown by functions in the LOLCode namespace
   *
   * This is what should be thrown by an error condition in any function
   * in the LOLCODE namespace.  It should contain a reference to what function
   * generated the error, what line of the input file the error was generated
   * from, and a reference to what parser entity generated the error.
   *   This should also probably implement a trace through the levels that
   * it gets re-thrown through.
   */
  class HookError
  {
    public:
      string         error; /*!< The error that occured */
      unsigned       line;  /*!< The line on which the error occured */
      vector<string> calls; /*!< Error backtrace */

      HookError(string e, string r, unsigned l); /*!< Make a HookError */
      HookError(string full_error); /*!< Make a HookError that's not specifically line-based */

      void called_by(string e, unsigned l); /*!< Push a call onto the end of the call stack */

      operator string() const; /*!< Convert this into a string */
      string to_string() const; /*!< Get the representation of this as a string */
      string backtrace() const; /*!< Get the backtrace of this error as a string */
  };

  /*!
   * \brief Searh for a hook
   */
  HookFunc hook_search(string id);

  /*!
   * \brief Convert anything to anything
   */
  template <typename IN, typename OUT>
  OUT convert(IN i);
}

#endif
