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
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  HookReturn program(ASTNode *node) 
  { 
    HookReturn r = {"",NULL};
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      cout << "HAI" << endl;
     
      if (!node->terminal)
      {
        for (unsigned i = 0; i < node->nodecount; ++i)
        {
          ASTNode *child = (ASTNode*)node->nodes[i];
          string child_rule = type_names[child->type];
          hook_search(child_rule)(child);
        }
      }

      cout << "KTHXBYE" << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
    return r;
  }

  /*!
   * \brief Run an assignment
   *
   * This handles assignment of variables (and declaration as well)
   * Children:
   *  0. Loop label
   *  1. Statements
   * 
   * \param node The node to traverse
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  HookReturn assignment(ASTNode *node) 
  { 
    HookReturn r = {"",NULL};
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      ASTNode *l_value = (ASTNode*)node->nodes[0];
      ASTNode *r_value = (ASTNode*)node->nodes[1];

      cout << "    LOL " << std::flush;
      hook_search( type_names[l_value->type] )(l_value);
      cout << " R " << std::flush;
      hook_search( type_names[r_value->type] )(r_value);
      cout << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
    return r;
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
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  HookReturn loop(ASTNode *node) 
  { 
    HookReturn r = {"",NULL};
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      ASTNode *label = (ASTNode*)node->nodes[0];
      ASTNode *inner = (ASTNode*)node->nodes[1];

      cout << "  IM IN YR " << (char *)(label->nodes[0]) << endl;
      hook_search( type_names[inner->type] )(inner);
      cout << "  KTHX" << endl;
      free(label->nodes[0]);
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
    return r;
  }

  /*!
   * \brief Call blindly all of the nodes linked to this one.
   *
   * Iterate through all of the children nodes attached to this one and call
   * them. This function does not append itself to the call stack (or it won't).
   * 
   * \param node The node to traverse
   * \return The standard hook return
   * \throw HookError If a sub-node is not a recognized type (i.e. can't be executed)
   * \throw HookError If the node is a terminal node
   */

  HookReturn fork(ASTNode *node) 
  { 
    HookReturn r = {"",NULL};
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    if (!node->terminal)
    {
      for (unsigned i = 0; i < node->nodecount; ++i)
      {
        ASTNode *child = (ASTNode*)node->nodes[i];
        string child_rule = type_names[child->type];
        line = child->lineno;
        hook_search( type_names[child->type] )(child);
      }
    }
    else
    {
      throw HookError("Node is a terminal node!", rule, line);
    }
    return r;
  }

  /*!
   * \brief Does nothing.
   */
  HookReturn noop(ASTNode *node)
  {
    HookReturn r = {"",NULL};
    cout << "  BTW Noop" << endl;
    return r;
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
