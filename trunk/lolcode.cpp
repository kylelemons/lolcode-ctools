#include <string>

#include "lolcode.hpp"
#include "ast.h"

namespace LOLCode
{
  using std::string;

  hook_func hook_search(string id)
  {
    unsigned i;
    for (i = 0; hooks[i].id != NULL && hooks[i].func != NULL; ++i)
    {
      if (id == hooks[i].id)
        break;
    }
    return hooks[i].func;
  }

  hook_return program(ast_node *node) 
  { 
    hook_return r = {"",NULL};
    return r;
  }

  const hook hooks[] = {
    { "program", program },
    { NULL, NULL }
  };
}
