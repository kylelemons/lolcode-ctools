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
      unsigned int indent; /*!< Holds the current output indentation. =) */
      unsigned int counter; /*!< This counter remains unique and should only increment */

      map<string,bool> flags; /*!< Holds various flags that should persist */
      vector<string> file_lines; /*!< Holds the source of the output program */
      map<string,string> symbols; /*!< Holds the symbols we're using and their types */
      map<string,string> variables; /*!< Holds the variables that we're using and their types */
      map<string, vector<int> > dimensions; /*!< Holds the sizes of the arrays we're using */

      stack<int> int_stack;
      stack<string> string_stack;
      stack<string> context_stack;

      CompilerContext();
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
