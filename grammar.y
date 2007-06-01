%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// abstract syntax tree
#include "ast.h"

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
int yywrap()
{
        return 1;
} 

// Token string
#define TS(x) type_names[x]
// Token number
#define TN ((yyr1[yyn]<=YYNTOKENS)?yyr1[yyn]:printf("LINE:%d->%d(on)%d\n",yyn,yyr1[yyn],__LINE__),yyr1[yyn])
#undef TN
#define TN (printf("\n{LINE=%d:yyn=%d:yystos=%d:yyr1=%d:yytname=%s,yytname2=%s}\n",__LINE__,yyn,yystos[yyn],yyr1[yyn],yytname[yyr1[yyn]],yytname[yyn]),yyn)
#undef TN
#define TN yyr1[yyn]

ast_node *root = NULL;

void *idup(int x)
{
  int *r = (int *)malloc(sizeof(int));
  *r = x;
  return r;
}

void ws(unsigned indent)
{
  unsigned i;
  for (i = 0; i < indent; ++i) printf(" ");
}

void print_tree(ast_node *node, unsigned indent)
{
  if (!node)
  {
    ws(indent); printf("<NULL>\n");
    return;
  }
  if (!TS(node->type))
  {
    ws(indent); printf("Huh? TN = %d\n", node->type);
  }
  ws(indent); printf("{%s:%d:%p}\n", TS(node->type), node->nodecount, node->nodes);
  if (node->terminal == 0)
  {
    unsigned n;
    for (n = 0; n < node->nodecount; ++n)
    {
      if (node->nodes[n])
      {
        print_tree(node->nodes[n], indent+2);
      }
      else
        ws(indent+2),printf("Uh ho! NULL\n");
    }
  }
  //ws(indent+2),printf("<terminal>\n");
}

%}


%union {
  int   num;
  char *str;
  struct ast_node_t *node;
}

%token <num> T_NUMBER 
%token <str> T_WORD T_STRING
%token <num> P_EXCL NEWLINE
%token <num> A AND BIGR BYES CAN COMMENT DIAF GIMMEH GTFO HAI HAS 
%token <num> I IM IN ITZ IZ KTHX KTHXBYE LIEK LETTAR LINE LOL MAH 
%token <num> NERF NERFZ NOT NOWAI OR OUTTA OVAR OVARZ R 
%token <num> SMALR STDIN THAN TIEMZ TIEMZD UP UPZ VISIBLE 
%token <num> WORD XOR YARLY YR P_QMARK

%type <node> array array_index assignment condexpr conditional
%type <node> declaration exit exit_status exit_message expr 
%type <node> include increment_expr initializer input_type input_from input 
%type <node> l_value loop loop_label output program
%type <node> self_assignment stmt stmts

%expect 102

%start program

%%
program : prog_start stmts prog_end { printf("Valid program!\n"); $$ = CN(TN); AL($$,$2); root = $$; }
;

array : array_index array  { $$ = CN(TN); ALL($$,$1,$2); } 
      | T_WORD             { printf("[VAR:%s]", $1); $$ = CT(TN); AL($$,$1); }
;

/* TODO: Figure out how to allow an expr as the index... */
array_index : T_NUMBER IN MAH { $$ = CT(TN); AL($$,idup($1)); }
            | T_WORD IN MAH { $$ = CT(TN); AL($$,$1); }
;

assignment : LOL l_value R expr     { $$ = CN(TN); ALL($$,$2,$4); }
           | self_assignment      { $$ = CN(TN); }
;

condexpr : expr BIGR THAN expr      { printf("[>]"); $$ = CN(TN); ALL($$,$1,$4); }
         | expr NOT BIGR THAN expr  { printf("[<=]"); $$ = CN(TN); ALL($$,$1,$5); }
         | expr SMALR THAN expr     { printf("[<]"); $$ = CN(TN); ALL($$,$1,$4); }
         | expr NOT SMALR THAN expr { printf("[>=]");  $$ = CN(TN); ALL($$,$1,$5); }
         | expr LIEK expr           { printf("[==]");  $$ = CN(TN); ALL($$,$1,$3); }
         | expr NOT LIEK expr       { printf("[!=]");  $$ = CN(TN); ALL($$,$1,$4); }
         | condexpr OR condexpr     { printf("[||]");  $$ = CN(TN); ALL($$,$1,$3); }
         | condexpr AND condexpr    { printf("[&&]");  $$ = CN(TN); ALL($$,$1,$3); }
         | condexpr XOR condexpr    { printf("[^^]");  $$ = CN(TN); ALL($$,$1,$3); }
         | NOT condexpr             { printf("[!]");  $$ = CN(TN); AL($$,$2); }
;

conditional : IZ condexpr then stmts KTHX     { $$ = CN(TN); ALL($$,$2,$4); }
            | IZ condexpr then stmts elsethen stmts KTHX     { $$ = CN(TN); ALLL($$,$2,$4,$6); }
;

declaration : I HAS A array initializer     { $$ = CN(TN); ALL($$,$4,$5); }
;

elsethen : NOWAI end_stmt
;

end_stmt : NEWLINE           { printf("\n"); }
;

exit : DIAF exit_status exit_message     { $$ = CN(TN); ALL($$,$2,$3); }
     | BYES exit_status exit_message     { $$ = CN(TN); ALL($$,$2,$3); }
;

exit_status : /* nothing */ { $$ = CT(TN); }
            | T_NUMBER      { $$ = CT(TN); AL($$,idup($1)); }
            | T_WORD        { $$ = CT(TN); AL($$,$1); }
;

exit_message : /* nothing */ { $$ = CT(TN); }
             | T_STRING      { $$ = CT(TN); AL($$,$1); }
             | T_WORD        { $$ = CT(TN); AL($$,$1); }
;

expr : T_NUMBER          { printf("[NUM:%d]", $1);  $$ = CT(TN); AL($$,idup($1)); }
     | array             { $$ = CN(TN); AL($$,$1); }
     | T_STRING          { printf("[STR:%s]", $1);  $$ = CT(TN); AL($$,$1); }
     | expr UP expr      { printf("[+]");  $$ = CN(TN); ALL($$,$1,$3); }
     | expr NERF expr    { printf("[-]");  $$ = CN(TN); ALL($$,$1,$3); }
     | expr TIEMZ expr   { printf("[*]");  $$ = CN(TN); ALL($$,$1,$3); }
     | expr OVAR expr    { printf("[/]");  $$ = CN(TN); ALL($$,$1,$3); }
;

include : CAN HAS T_WORD P_QMARK   { $$ = CT(TN); AL($$,$3); }
        | CAN HAS T_STRING P_QMARK { $$ = CT(TN); AL($$,$3); }

increment_expr : /* empty (defaults to 1) */     { $$ = CT(TN); }
               | expr     { $$ = CN(TN); AL($$,$1); }
;

initializer: ITZ expr     { $$ = CN(TN); AL($$,$2); }
           | /* empty */  { $$ = CN(TN); }
;

input_type : /* empty */      { $$ = CT(TN); }
           | WORD     { $$ = CT(TN); }
           | LINE     { $$ = CT(TN); }
           | LETTAR     { $$ = CT(TN); }
;

input_from : /* empty */      { $$ = CT(TN); }
           | OUTTA T_WORD     { $$ = CT(TN); AL($$,$2); }
           | OUTTA STDIN     { $$ = CT(TN); }
;

input : GIMMEH input_type array input_from { $$ = CN(TN); ALLL($$,$2,$3,$4); }
;

l_value : array     { $$ = CN(TN); AL($$,$1); }
;

loop : IM IN YR loop_label end_stmt stmts KTHX     { $$ = CN(TN); ALL($$,$4,$6); }
;

loop_label : T_WORD  { $$ = CT(TN); }
;

output : VISIBLE expr     { $$ = CT(TN); AL($$,$2); }
       | VISIBLE expr P_EXCL     { $$ = CT(TN); AL($$,$2); }
;

prog_start : HAI end_stmt
;

prog_end   : KTHXBYE
           | prog_end end_stmt
;

self_assignment : UPZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN); ALL($$,$2,$5); }
                | NERFZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN); ALL($$,$2,$5); }
                | TIEMZD l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN); ALL($$,$2,$5); }
                | OVARZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN); ALL($$,$2,$5); }
;

stmt : include               { printf("Inclusion");  $$ = CN(TN); AL($$,$1); }
     | declaration           { printf("Declaration");  $$ = CN(TN); AL($$,$1); }
     | loop                  { printf("Loop");  $$ = CN(TN); AL($$,$1); }
     | conditional           { printf("Conditional");  $$ = CN(TN); AL($$,$1); }
     | assignment            { printf("Assignment");  $$ = CN(TN); AL($$,$1); }
     | input                 { printf("Input");  $$ = CN(TN); AL($$,$1); }
     | output                { printf("Output");  $$ = CN(TN); AL($$,$1); }
     | expr                  { printf("Expression");  $$ = CN(TN); AL($$,$1); }
     | GTFO                  { printf("Break"); $$ = CT(TN); }
     | exit                  { printf("Exit");  $$ = CN(TN); AL($$,$1); }
     | COMMENT               { printf("/* COMMENT */"); $$ = CT(TN); }
;

stmts : /* No statements at all */     { $$ = CT(TN); }
      | stmts end_stmt /* empty line */     { $$ = CN(TN); AL($$,$1); }
      | stmt end_stmt            { $$ = CN(TN); AL($$,$1); }
      | stmts stmt end_stmt      { $$ = CN(TN); ALL($$,$2,$1); }
;

then : end_stmt
     | P_QMARK end_stmt
     | end_stmt YARLY
     | P_QMARK end_stmt YARLY
;

%%
int main(int argc, char **argv)
{
  type_names = yytname;

  yyparse();

  printf("Parse tree:\n");
  print_tree(root, 0);

  /*
  unsigned i;
  for (i = 0; i < YYNTOKENS; ++i)
   if (yytname[i])
    printf("yytname[%d] = \"%s\" (KTHXBYE is %d);\n", i, yytname[i], KTHXBYE);
  */

  return 0;
} 
