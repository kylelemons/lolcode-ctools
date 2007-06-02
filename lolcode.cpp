#include <string>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
#include <iomanip>
#include <cmath>

#include "lolcode.hpp"
#include "ast.h"

namespace LOLCode
{
  using std::string;

  CompilerContext::CompilerContext()
    : indent(0)
  {
  }

  /*!
   * \param i The input value
   * \return The converted value
   */
  template <typename IN, typename OUT>
  OUT convert(IN i)
  {
    std::stringstream ss;
    OUT o;
    ss << i;
    ss >> o;
    return o;
  }

  /*!
   * \param e The error
   * \param r The rule
   * \param l The line on which the error occured
   */
  HookError::HookError(string e, string r, unsigned l)
    : error(e), line(l)
  {
    called_by(r,l);
  }

  /*!
   * \param fullerror The full error message
   */
  HookError::HookError(string fullerror)
    : error(fullerror), line(-1)
  {
  }

  /*!
   * \param e The error
   * \param l The line on which the caller exists
   */
  void HookError::called_by(string e, unsigned l)
  {
    std::stringstream ss;
    ss << e << " (line #" << l << ")";
    calls.push_back(ss.str());
  }

  /*!
   * Call the to_string method and return the results.  Useful for
   * quickly printing this out, i.e.:
   *   cout << "Error:" << (string)e << endll
   */
  HookError::operator string() const
  {
    return to_string();
  }

  /*!
   * Return the first item on the call stack (the generating caller)
   * as the string equivalent of this error.
   */
  string HookError::to_string() const
  {
    return error;
  }

  /*!
   *   Return a string with the call stack of this error.  This is really handy
   * for the person who has to try to debug the output of this =D
   */
  string HookError::backtrace() const
  {
    std::stringstream ss;
    int numwidth = int(log(calls.size()+1)) + 1;
    for (unsigned i = 0; i < calls.size(); ++i)
    {
      ss << " " << std::setw(numwidth) << (i+1) << ": " << calls[i] << endl;
    }
    return ss.str();
  }

  /*!
   * Search through the hooks and return a function that implements that hook
   * \param id The name of the hook you're looking for
   * \return a hook function
   * \throw HookError If the hook is not found
   */
  HookFunc hook_search(string id)
  {
    unsigned i;
    for (i = 0; hooks[i].id != NULL && hooks[i].func != NULL; ++i)
    {
      if (id == hooks[i].id)
        break;
    }
    if (hooks[i].id == NULL)
      throw HookError("Unable to locate hook for: " + id);
    return hooks[i].func;
  }

  /*!
   * \brief Outer program block
   *
   * This handles the generalized program set-up and destruction.
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void program(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      cout << "HAI" << endl;
      context.context_stack.push("global");
     
      if (!node->terminal)
      {
        for (unsigned i = 0; i < node->nodecount; ++i)
        {
          ASTNode *child = (ASTNode*)node->nodes[i];
          string child_rule = type_names[child->type];
          hook_search(child_rule)(child, context);
        }
      }

      context.context_stack.pop();
      cout << "KTHXBYE" << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Access an array
   *
   * This handles accessing arrays, and expanding them if necessary
   * Children:
   *  0. array_index
   *  1. array
   *   -or-
   *  0. T_WORD
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void array(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      if (node->nodecount == 1)
      { // straight-up array
      }
      else if (node->nodecount == 2)
      { // sub-indexed array
        //ASTNode *array_index = (ASTNode*)node->nodes[0];
        //ASTNode *array = (ASTNode*)node->nodes[1];

      }
      context.flags["r_value"] = true;
      //hook_search( type_names[l_value->type] )(l_value, context);
      context.flags["r_value"] = false;
      //cout << " R " << std::flush;
      context.flags["l_value"] = true;
      //hook_search( type_names[r_value->type] )(r_value, context);
      context.flags["l_value"] = false;
      //cout << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Run an assignment
   *
   * This handles assignment of variables (and declaration as well)
   * Children:
   *  0. l_value
   *  1. r_value
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void assignment(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      ASTNode *l_value = (ASTNode*)node->nodes[0];
      ASTNode *r_value = (ASTNode*)node->nodes[1];

      cout << "    LOL " << std::flush;
      context.flags["r_value"] = true;
      hook_search( type_names[l_value->type] )(l_value, context);
      context.flags["r_value"] = false;
      cout << " R " << std::flush;
      context.flags["l_value"] = true;
      hook_search( type_names[r_value->type] )(r_value, context);
      context.flags["l_value"] = false;
      cout << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Run a loop
   *
   * This handles the infinite looping mechanism
   * Children:
   *  0. Loop label
   *  1. Statements
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void loop(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      context.context_stack.push("loop"+convert<int,string>(context.counter++));
      ASTNode *label = (ASTNode*)node->nodes[0];
      ASTNode *inner = (ASTNode*)node->nodes[1];

      cout << "  IM IN YR " << (char *)(label->nodes[0]) << endl;
      hook_search( type_names[inner->type] )(inner, context);
      cout << "  KTHX" << endl;
      context.context_stack.pop();
      free(label->nodes[0]);
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Call blindly all of the nodes linked to this one.
   *
   * Iterate through all of the children nodes attached to this one and call
   * them. This function does not append itself to the call stack (or it won't).
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError If a sub-node is not a recognized type (i.e. can't be executed)
   * \throw HookError If the node is a terminal node
   */

  void fork(ASTNode *node, CompilerContext &context)
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    if (!node->terminal)
    {
      for (unsigned i = 0; i < node->nodecount; ++i)
      {
        ASTNode *child = (ASTNode*)node->nodes[i];
        string child_rule = type_names[child->type];
        line = child->lineno;
        hook_search( type_names[child->type] )(child, context);
      }
    }
    else
    {
      throw HookError("Node is a terminal node!", rule, line);
    }
  }

  /*!
   * \brief Does nothing.
   */
  void noop(ASTNode *node, CompilerContext &context)
  {
    cout << "  BTW Noop" << endl;
  }

  const Hook hooks[] = {
    { "assignment", assignment },
    { "declaration", assignment },
    { "include", noop },
    { "loop", loop },
    { "loops", fork },
    { "program", program },
    { "stmt", fork },
    { "stmts", fork },
    { NULL, NULL }
  };
}
