#ifndef LOLCODE_H
#define LOLCODE_H

#include <string>

#include "ast.h"

namespace LOLCode
{
  using std::string;

  typedef struct _hook_ret {
    string type;
    void *ptr;
  } hook_return;

  typedef struct _hook_ret (*hook_func)(ast_node *node);

  typedef struct _hook {
    const char *id;
    hook_func func;
  } hook;

  extern const hook hooks[];

  hook_func hook_search(string id);
}

#endif
