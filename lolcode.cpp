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
   * \param l The line on which the error occured
   */
  HookError::HookError(string e, unsigned l)
    : error(e), line(l)
  {
    called_by(e,l);
  }

  /*!
   * \param fullerror The full error message
   */
  HookError::HookError(string fullerror)
    : error(fullerror), line(-1)
  {
    calls.push_back(fullerror);
  }

  /*!
   * \param e The error
   * \param l The line on which the caller exists
   */
  void HookError::called_by(string e, unsigned l)
  {
    std::stringstream ss;
    ss << e << "/" << l;
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
    return calls[0];
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
   * \throw Throws an instance of hook_error if no hook is found
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

  HookReturn program(ASTNode *node) 
  { 
    HookReturn r = {"",NULL};
    cout << "HAI" << endl;

    cout << "KTHXBYE" << endl;
    return r;
  }

  const Hook hooks[] = {
    { "program", program },
    { NULL, NULL }
  };
}
