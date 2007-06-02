#ifndef LOLCODE_H
#define LOLCODE_H

#include <string>
#include <vector>

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

  /*!
   * \brief Return value from a Hook function.
   */
  typedef struct {
    string type;                  /*!< The type stored at the void */
    void *ptr;                    /*!< A pointer to the type described by type */
  } HookReturn;

  /*! \brief The Abstract Syntax Tree (A.S.T) Node */
  typedef ast_node ASTNode;
  /*! \brief The types of functions stored in hooks */
  typedef HookReturn (*HookFunc)(ASTNode *node);

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
}

#endif
