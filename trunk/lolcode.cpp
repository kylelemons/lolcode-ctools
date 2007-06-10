#include <string>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
#include <iomanip>
#include <cmath>

#include "lolcode.hpp"
#include "asmutil.h"
#include "ast.h"

namespace LOLCode
{
  using std::string;

  /*!
   * \brief Constructor
   */

  CompilerContext::CompilerContext()
    : counter(0), filename("stdin")
  {
  }

  const string CompilerContext::ret_reg("%eax");
  const string CompilerContext::var_reg("%ebx");
  const string CompilerContext::cnt_reg("%ecx");
  const string CompilerContext::val_reg("%edx");
  const string CompilerContext::dim_reg("%esi");
  const string CompilerContext::ptr_reg("%edi");
  const string CompilerContext::frame_ptr("%ebp");
  const string CompilerContext::stack_ptr("%esp");

  /*!
   * \brief Append a newline and queue for output
   *
   * Suffix a comment and add the line with a newline to the data
   * that will be written to the file in the "program" section
   *
   * \param piece The piece to be written to the file with newline and comment
   * \param lineno The line number of the *input file* which generated this line
   */

  void CompilerContext::output(string piece, string comment)
  {
    string line;
    line += string( context_stack.size()*2, ' ' ); // indent
    line += piece;
    if (comment.size() > 0)
    {
      line += string( 3, ' ' );
      line += string( tab_width - (line.size()%tab_width), ' ' );
      line += "# " + comment;
    }
    output_raw(line + "\n");
  }

  /*!
   * \brief Append a newline and queue for output (with debugging info)
   *
   * Suffix a comment and add the line with a newline to the data
   * that will be written to the file in the "program" section
   *
   * \param piece The piece to be written to the file with newline and comment
   * \param lineno The line number of the *input file* which generated this line
   */

  void CompilerContext::output(string piece, unsigned lineno)
  {
    string comment;
    if (lineno > 0)
    {
      comment += "<- [";
      comment += filename;
      comment += ":";
      comment += convert<unsigned,string>(lineno);
      comment += "]";
    }
    output(piece, comment);
  }

  /*!
   * \brief Queue piece for output verbatim
   *
   * Suffix no comment nor newline and add the line with a newline to the data
   * that will be written to the file in the "program" section
   *
   * \param piece The piece to be written to the file with no processing
   */

  void CompilerContext::output_raw(string piece)
  {
    file_pieces.push_back(piece);
  }

  /*!
   * \brief Append a newline and queue for output to the header (with debugging info)
   *
   * Suffix a comment and add the line with a newline to the data
   * that will be written to the file in the "header" section
   *
   * \param piece The piece to be written to the file with newline and comment
   * \param lineno The line number of the *input file* which generated this line
   */

  void CompilerContext::header(string piece, unsigned lineno)
  {
    string line;
    line += string( context_stack.size()*2, ' ' ); // indent
    line += piece;
    if (lineno > 0)
    {
      line += string( 3, ' ' );
      line += string( tab_width - (line.size()%tab_width), ' ' );
      line += "# <- [";
      line += filename;
      line += ":";
      line += convert<unsigned,string>(lineno);
      line += "]";
    }
    header_raw(line + "\n");
  }

  /*!
   * \brief Queue piece for output to the header verbatim
   *
   * Suffix no comment nor newline and add the line with a newline to the data
   * that will be written to the file in the "program" section
   *
   * \param piece The piece to be written to the file with no processing
   */

  void CompilerContext::header_raw(string piece)
  {
    header_pieces.push_back(piece);
  }

  /*!
   * \brief Return the contents of the output file
   *
   * Stick all of the pieces together into one string suitable for writing
   * to the output file and return it
   */

  string CompilerContext::build_file()
  {
    string output;
    for (unsigned i = 0; i < header_pieces.size(); ++i)
      output += header_pieces[i];
    output += "\n";
    for (unsigned i = 0; i < file_pieces.size(); ++i)
      output += file_pieces[i];
    return output;
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
      context.header(".section .data");
      context.output(".section .text");
      context.output(".globl main");
      context.output("main:", line);
      cout << "HAI" << endl;
      context.context_stack.push("program");
      context.varcontext_stack.push("global");
     
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
      context.output("movl $1, %eax", line);
      context.output("movl $0, %ebx");
      context.output("int $0x80");

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
   *  0. array
   *  1. expr
   *   -or-
   *  0. word
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return pushes: {S} arr_name, {I} dim#, dim1min, dim1max, ... dimNmin, dimNmax
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void array(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    string ctext = context.varcontext_stack.top();
    try
    {
      string varname = "";
      ASTNode *firstnode = (ASTNode*)(node->nodes[0]);
      //cout << "Type1: " << type_names[ firstnode->type ] << endl;
      if (strcmp( type_names[ firstnode->type ], "word") == 0)
      { // straight-up array
        char *varname = (char*)((ASTNode*)node->nodes[0])->nodes[0];
        bool need_registers = true;
        //cout << "Variable: " << varname << endl;
        cout << varname << endl;
        if (context.flags["r_value"] == true)
        {
          if (context.variables[ctext].find(varname) == context.variables[ctext].end())
          {
            // allocate the new variable
            context.output("push $1", line); // dimension count
            context.output("push $" + convert<int,string>(TYPE_IDK)); // type
            context.output("call varalloc", string(varname)); // allocate var
            context.output("addl $8, " + context.stack_ptr); // pop params from stack
            context.output("movl " + context.ret_reg + ", " + context.var_reg); // move return value into the variable register
            context.output("push $1"); // push new length
            context.output("push $0"); // push dimension
            context.output("push " + context.var_reg); // push pointer
            context.output("call vardimalloc"); // allocate dimension
            context.output("pop " + context.var_reg); // pop off the arguments (just in case it was clobbered, I guess)
            context.output("addl $8, " + context.stack_ptr); // pop off the rest of the arguments
            context.output("movl 12(" + context.var_reg + "), " + context.dim_reg);
            context.output("push " + context.var_reg, "Store " + string(varname));
            need_registers = false;
            context.mem_stack[ctext] -= 4; // allocate the size of a pointer on the stack
            for (map<string, int>::iterator iter = context.offset[ctext].begin(); iter != context.offset[ctext].end(); ++iter)
            {
              iter->second += 4;
              //cout << "After stack decrement, " << iter->first << " is now at " << iter->second << "(" << context.stack_ptr << ")" << endl;
            }
            context.offset[ctext][varname] = 0;
            //cout << "After stack decrement, " << varname << " is now at " << 0 << "(" << context.stack_ptr << ")" << endl;

            //context.variables[ctext][varname] = varname + convert<unsigned,string>(line);
            context.variables[ctext][varname] = "IDK";
            context.dimensions[ctext][varname].push_back(1);
          }
        }
        if (context.variables[ctext].find(varname) == context.variables[ctext].end())
        {
          throw HookError("No such variable: " + string(varname));
        }
        // Load up the variable as we'll need it
        if (need_registers)
        {
          context.output("movl " + convert<int,string>(context.offset[ctext][varname]) + "(" + context.stack_ptr + "), " + context.var_reg, line);
          context.output("movl 12(" + context.var_reg + "), " + context.dim_reg, varname);
          context.output("push $0");
          context.output("push " + context.dim_reg);
          context.output("call validx");
          context.output("addl $8, " + context.stack_ptr);
          context.output("movl " + context.ret_reg + ", " + context.dim_reg);
          context.output("movl (" + context.dim_reg + "), " + context.ret_reg);
        } 
        // return (REMEMBER: Backwards of what's popped!)
        context.string_stack.push(varname);
        for (int i = context.dimensions[ctext][varname].size()-1; i >= 0; --i)
        {
          context.int_stack.push( context.dimensions[ctext][varname][i] ); // dimNmax
          context.int_stack.push( 0 ); // dimNmin
        }
        context.int_stack.push( context.dimensions[ctext][varname].size() );
        //cout << "Done with array branch 1!" << endl;
      }
      else
      { // sub-indexed array
        // Get the name of the array
        cout << "SUB INDEXED ARRAY (warning, not debugged!)" << endl;
        int dims, idxn;
        vector<int> mindims, maxdims;
        ASTNode *array = (ASTNode*)node->nodes[0];
        hook_search( type_names[array->type] )(array,context);

        // Get the return value
        varname = context.string_stack.top(); context.string_stack.pop();
        idxn = context.int_stack.top();       context.int_stack.pop();
        dims = context.int_stack.top();       context.int_stack.pop();
        for (int i = 0; i < dims; ++i)
        {
          mindims.push_back( context.int_stack.top() ); context.int_stack.pop();
          maxdims.push_back( context.int_stack.top() ); context.int_stack.pop();
        }

        ASTNode *array_index = (ASTNode*)node->nodes[1];
        hook_search( type_names[array_index->type] )(array_index,context); // stores in eax
        context.output("movl " + convert<int,string>(context.offset[ctext][varname]) + "(" + context.stack_ptr + "), " + context.var_reg, line);
        context.output("movl 12(" + context.var_reg + "), " + context.dim_reg);
        context.output("push " + context.ret_reg, "expr");
        context.output("push " + context.dim_reg, varname);
        context.output("call validx");
        context.output("addl $4, " + context.stack_ptr);
        context.output("movl " + context.ret_reg + ", " + context.dim_reg);
        context.output("movl (" + context.dim_reg + "), " + context.ret_reg);
      }
      cout << "Done with array!" << endl;
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

      cout << string(context.context_stack.size()*2, ' ') << "LOL " << std::flush;
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
   * \brief Break out of a loop
   *
   * This handles assignment of variables (and declaration as well)
   * Children:
   *  - none -
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if there is no enclosing loop* context
   */

  void brk(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    string ctext;
    { // Find the top-most context with loop in it
      stack<string> temp;
      while (context.context_stack.empty() == false)
      {
        if (context.context_stack.top().find("loop") == 0) // loop context
          break;
        temp.push( context.context_stack.top() );
        context.context_stack.pop();
      }
      if (context.context_stack.empty())
        throw HookError("BREAK found outside of loop!", rule, line);
      else
        ctext = context.context_stack.top();
      while (temp.empty() == false)
      {
        context.context_stack.push( temp.top() );
        temp.pop();
      }
    }
    cout << string(context.context_stack.size()*2, ' ') << "BTW Break from " << ctext << std::endl;
    cout << string(context.context_stack.size()*2, ' ') << "GTFO" << std::endl;
  }

  /*!
   * \brief Evaluate a conditional
   *
   * This handles the conditional statements
   * Children:
   *  0. condition
   *  1. then-statements
   *  2. (optionaal) else-statements
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void conditional(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    string ctext = "cond"+convert<int,string>(context.counter++);
    try
    {
      ASTNode *cond = (ASTNode*)node->nodes[0];
      ASTNode *tbranch = (ASTNode*)node->nodes[1];
      ASTNode *ebranch = (ASTNode*)node->nodes[2];

      cout << string( context.context_stack.size()*2, ' ');
      cout << "IZ " << std::flush;
      context.context_stack.push(ctext);
      hook_search( type_names[cond->type] )(cond, context);
      context.context_stack.pop();
      cout << endl;

      context.context_stack.push(ctext+"then");
      hook_search( type_names[tbranch->type] )(tbranch, context);
      context.context_stack.pop();

      if (node->nodecount == 3)
      {
        cout << string( context.context_stack.size()*2, ' ');
        cout << "NOWAI" << endl;

        context.context_stack.push(ctext+"else");
        hook_search( type_names[ebranch->type] )(ebranch, context);
        context.context_stack.pop();
      }


      cout << string( context.context_stack.size()*2, ' ');
      cout << "KTHX" << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Evaluate a conditional expression
   *
   * This handles the conditional expressions
   * Children:
   *  0. boolean
   *   -or-
   *  0. unary op
   *  1. conditional expression
   *   -or-
   *  0. binary op
   *  1. conditional expression
   *  2. conditional expression
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void condexpr(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    string ctext = context.context_stack.top(); // know where to jump
    try
    {
      ASTNode *op = (ASTNode*)node->nodes[0];
      ASTNode *c1 = (ASTNode*)node->nodes[1];
      ASTNode *c2 = (ASTNode*)node->nodes[2];

      if (node->nodecount == 1) // strict boolean literal
      {
        bool boolean = ( *(int*)(op) ) == 1;
        if (boolean)
          cout << "WIN" << std::flush;
        else
          cout << "FAIL" << std::flush;
      }
      else if (node->nodecount == 2) // unary operator
      {
        char unary = *(char*)(op);
        switch (unary)
        {
          case '!':
            cout << "NOT " << std::flush; break;
        }
        hook_search( type_names[c1->type] )(c1, context);
      }
      else if (node->nodecount == 3) // binary operator
      {
        char binary = *(char*)(op);
        switch (binary)
        {
          case '>':
            cout << "BIGR " << std::flush; break;
          case '<':
            cout << "SMALR " << std::flush; break;
          case '=':
            cout << "LIEK " << std::flush; break;
          case '|':
            cout << "OR " << std::flush; break;
          case '&':
            cout << "AND " << std::flush; break;
          case '^':
            cout << "XOR " << std::flush; break;
        }
        hook_search( type_names[c1->type] )(c1, context);
        cout << " AN " << std::flush;
        hook_search( type_names[c2->type] )(c2, context);
      }
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }


  /*!
   * \brief Evaluate a constant
   *
   * This handles assignment of variables (and declaration as well)
   * Children:
   *  0. value
   *    - Valid types:
   *      a. number
   *      b. string
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void constant(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    string number_type = "number";
    string string_type = "string";
    try
    {
      ASTNode *value = node;
      if (number_type == type_names[value->type])
      {
        cout << *(int*)(value->nodes[0]) << std::flush;
      } // number constant
      else
      {
        cout << "\"" << (char*)(value->nodes[0]) << std::flush;
      } // string constant
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Evaluate an expression
   *
   * This handles accessing arrays, and expanding them if necessary
   * Children:
   *  0. operation
   *  1. expression
   *  2. expression
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   * \throw HookError If somehow this gets called with something that's not wellformed
   */

  void expr(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    //string ctext = context.context_stack.top(); // know where to jump
    try
    {
      ASTNode *op = (ASTNode*)node->nodes[0];
      ASTNode *c1 = (ASTNode*)node->nodes[1];
      ASTNode *c2 = (ASTNode*)node->nodes[2];

      if (node->nodecount == 3) // binary operator
      {
        char binary = *(char*)(op);
        switch (binary)
        {
          case '+':
            cout << "UP " << std::flush; break;
          case '-':
            cout << "NERF " << std::flush; break;
          case '*':
            cout << "TIEMZ " << std::flush; break;
          case '/':
            cout << "OVAR " << std::flush; break;
        }
        hook_search( type_names[c1->type] )(c1, context);
        cout << " AN " << std::flush;
        hook_search( type_names[c2->type] )(c2, context);
      }
      else
        throw HookError("Binary operator expected (requires three sub-nodes)", rule, line);
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
      ASTNode *label = (ASTNode*)node->nodes[0];
      ASTNode *inner = (ASTNode*)node->nodes[1];

      cout << string( context.context_stack.size()*2, ' ');
      cout << "IM IN YR " << (char *)(label->nodes[0]) << endl;
      context.context_stack.push("loop"+convert<int,string>(context.counter++));
      hook_search( type_names[inner->type] )(inner, context);
      context.context_stack.pop();
      cout << string( context.context_stack.size()*2, ' ');
      cout << "KTHX" << endl;
      free(label->nodes[0]);
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Handle output
   *
   * This handles initializer statements
   * Children:
   *  0. expr
   *  1. newline-suppressor (optional)
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void output(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      ASTNode *expr = (ASTNode*)node->nodes[0];
      hook_search( type_names[expr->type] )(expr, context);
      if (node->nodecount == 2)
        cout << "!" << std::flush;
      cout << endl;
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Increment statement
   *
   * This handles self-assignment/increment statements
   * Children:
   *  0. expr
   *   -or-
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   * \throw HookError If it can't find the assignment or expr token number
   */

  void increment(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      unsigned long assign_id = 0, expr_id = 0;
      for (unsigned long i = 0; i < type_count; ++i)
      {
        if (strcmp(type_names[i], "assignment") == 0)
          assign_id = i;
        else if (strcmp(type_names[i], "expr") == 0)
          expr_id = i;
      }
      if (expr_id == 0)
        throw HookError("Could not find token id for \"expr\"", rule, line);
      if (assign_id == 0)
        throw HookError("Could not find token id for \"assignment\"", rule, line);
      // Build ASTNodes to execute the equivalent statements
      void *op = node->nodes[0];
      void *lv = node->nodes[1];
      void *iv = node->nodes[2];
      ASTNode *as_node = create_ast_node(assign_id, line, 0);
      ASTNode *ex_node = create_ast_node(expr_id, line, 0);
      append_leaf(as_node, lv);
      append_leaf(as_node, ex_node);
      append_leaf(ex_node, op);
      append_leaf(ex_node, lv);
      append_leaf(ex_node, iv);
      // Run it
      hook_search( "assignment" )(as_node, context);
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Increment statement
   *
   * This handles increment statements
   * Children:
   *  0. expr
   *   -or-
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void inc_expr(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      if (node->nodecount == 1)
      { // straight-up array
        ASTNode *expr = (ASTNode*)node->nodes[0];
        hook_search( type_names[expr->type] )(expr, context);
      }
      else if (node->nodecount == 0)
      { // sub-indexed array
        cout << "1" << std::flush;
      }
    }
    catch (HookError e)
    {
      e.called_by(rule,line);
      throw e;
    }
  }

  /*!
   * \brief Initializer statement
   *
   * This handles initializer statements
   * Children:
   *  0. expr
   *   -or-
   * 
   * \param node The node to traverse
   * \param context The compiler context
   * \return The standard hook return
   * \throw HookError if a sub-node is not a recognized type (i.e. can't be executed)
   */

  void initializer(ASTNode *node, CompilerContext &context) 
  { 
    string rule = type_names[node->type];
    unsigned line = node->lineno;
    try
    {
      if (node->nodecount == 1)
      { // straight-up array
        ASTNode *expr = (ASTNode*)node->nodes[0];
        hook_search( type_names[expr->type] )(expr, context);
      }
      else if (node->nodecount == 0)
      { // sub-indexed array
        cout << "ITZ IDK" << std::flush;
      }
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
    { "array", array },
    { "assignment", assignment },
    { "brk", brk },
    { "comment", noop },
    { "condexpr", condexpr },
    { "conditional", conditional },
    { "declaration", assignment },
    { "expr", expr },
    { "include", noop },
    { "initializer", initializer },
    { "increment_expr", inc_expr },
    { "loop", loop },
    { "loops", fork },
    { "number", constant },
    { "output", output },
    { "program", program },
    { "self_assignment", increment },
    { "stmt", fork },
    { "stmts", fork },
    { NULL, NULL }
  };
}
