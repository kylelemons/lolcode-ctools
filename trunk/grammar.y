%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// abstract syntax tree
#include "ast.h"

void yyerror(const char *str);
int yywrap();

// Token string
#define TS(x) type_names[x]
// Token number/line number
#define TN yyr1[yyn]
#define LN lineno

// Globals that we need ( :( )
ast_node *root = NULL;
unsigned long lineno = 0;

// Duplicate an integer for inclusion in the parse tree
void *idup(int x)
{
  int *r = (int *)malloc(sizeof(int));
  *r = x;
  return r;
}

%}


%union {
  int   num;
  char *str;
  struct ast_node_t *node;
  unsigned long ulong;
}

%token <num> T_NUMBER 
%token <str> T_WORD T_STRING
%token <num> P_EXCL
%token <ulong> NEWLINE
%token <ulong> AND BIGR_THAN BYES CAN_HAS COMMENT DIAF FAIL GIMMEH GTFO HAI
%token <ulong> I_HAS_A IM_IN_YR ITZ IZ KTHX KTHXBYE LIEK LETTAR LINE LOL
%token <ulong> NERF NERFZ NOT NOWAI OR OUTTA OVAR OVARZ R 
%token <ulong> STDIN TIEMZ TIEMZD UP UPZ VISIBLE WIN
%token <ulong> WORD XOR YARLY P_QMARK

%left UPZ NERFZ TIEMZD OVARZ R
%left NOT
%left AND OR XOR
%left BIGR_THAN SMALR_THAN LIEK
%left UP NERF
%left TIEMZ OVAR
%left IN_MAH

%type <node> array array_expr array_index assignment condexpr conditional
%type <node> declaration exit exit_status exit_message expr 
%type <node> include increment_expr initializer input_type input_from input 
%type <node> l_value loop loop_label output program
%type <node> self_assignment stmt stmts
%type <ulong> prog_start prog_end

%expect 78

%start program

%%
program : prog_start stmts prog_end { $$ = CN(TN,$1); AL($$,$2); root = $$; }
;

array : array_index array  { $$ = CN(TN,LN); ALL($$,$1,$2); } 
      | T_WORD             { $$ = CT(TN,LN); AL($$,$1); }
;

/*TODO: Figure out how to allow an expr as the index... */
/*
array_index : expr IN_MAH  { $$ = CN(TN,LN); AL($$,$1); }
*/
array_index : T_NUMBER IN_MAH { $$ = CT(TN,LN); AL($$,idup($1)); }
            | T_WORD IN_MAH { $$ = CT(TN,LN); AL($$,$1); }
;

assignment : LOL l_value R expr     { $$ = CN(TN,LN); ALL($$,$2,$4); }
           | self_assignment      { $$ = CN(TN,LN); }
;

condexpr : WIN                      { int b = 1; $$ = CT(TN,LN); AL($$,idup(b)); }
         | FAIL                     { int b = 0; $$ = CT(TN,LN); AL($$,idup(b)); }
         | expr BIGR_THAN expr      { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | expr NOT BIGR_THAN expr  { $$ = CN(TN,LN); ALL($$,$1,$4); }
         | expr SMALR_THAN expr     { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | expr NOT SMALR_THAN expr { $$ = CN(TN,LN); ALL($$,$1,$4); }
         | expr LIEK expr           { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | expr NOT LIEK expr       { $$ = CN(TN,LN); ALL($$,$1,$4); }
         | condexpr OR condexpr     { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | condexpr AND condexpr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | condexpr XOR condexpr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
         | NOT condexpr             { $$ = CN(TN,LN); AL($$,$2); }
;

conditional : IZ condexpr then stmts KTHX     { $$ = CN(TN,$1); ALL($$,$2,$4); }
            | IZ condexpr then stmts elsethen stmts KTHX     { $$ = CN(TN,$1); ALLL($$,$2,$4,$6); }
;

declaration : I_HAS_A array initializer     { $$ = CN(TN,LN); ALL($$,$2,$3); }
;

elsethen : NOWAI end_stmt
;

end_stmt : NEWLINE           { lineno = $1; }
;

exit : DIAF exit_status exit_message     { $$ = CN(TN,LN); ALL($$,$2,$3); }
     | BYES exit_status exit_message     { $$ = CN(TN,LN); ALL($$,$2,$3); }
;

exit_status : /* nothing */ { $$ = CT(TN,LN); }
            | T_NUMBER      { $$ = CT(TN,LN); AL($$,idup($1)); }
            | T_WORD        { $$ = CT(TN,LN); AL($$,$1); }
;

exit_message : /* nothing */ { $$ = CT(TN,LN); }
             | T_STRING      { $$ = CT(TN,LN); AL($$,$1); }
             | T_WORD        { $$ = CT(TN,LN); AL($$,$1); }
;

array_expr : array              { $$ = CN(TN,LN); AL($$,$1); }
           | array UP expr      { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array NERF expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array TIEMZ expr   { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array OVAR expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
;

expr : T_NUMBER          { $$ = CT(TN,LN); AL($$,idup($1)); }
     | T_STRING          { $$ = CT(TN,LN); AL($$,$1); }
     | expr UP expr      { $$ = CN(TN,LN); ALL($$,$1,$3); }
     | expr NERF expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
     | expr TIEMZ expr   { $$ = CN(TN,LN); ALL($$,$1,$3); }
     | expr OVAR expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
     | array_expr        { $$ = CN(TN,LN); AL($$,$1); }
;

include : CAN_HAS T_WORD P_QMARK   { $$ = CT(TN,LN); AL($$,$2); }
        | CAN_HAS T_STRING P_QMARK { $$ = CT(TN,LN); AL($$,$2); }

increment_expr : /* empty (defaults to 1) */     { $$ = CT(TN,LN); }
               | expr     { $$ = CN(TN,LN); AL($$,$1); }
;

initializer: ITZ expr     { $$ = CN(TN,LN); AL($$,$2); }
           | /* empty */  { $$ = CN(TN,LN); }
;

input_type : /* empty */      { $$ = CT(TN,LN); }
           | WORD     { $$ = CT(TN,LN); }
           | LINE     { $$ = CT(TN,LN); }
           | LETTAR     { $$ = CT(TN,LN); }
;

input_from : /* empty */      { $$ = CT(TN,LN); }
           | OUTTA T_WORD     { $$ = CT(TN,LN); AL($$,$2); }
           | OUTTA STDIN     { $$ = CT(TN,LN); }
;

input : GIMMEH input_type array input_from { $$ = CN(TN,LN); ALLL($$,$2,$3,$4); }
;

l_value : array     { $$ = CN(TN,LN); AL($$,$1); }
;

loop : IM_IN_YR loop_label end_stmt stmts KTHX     { $$ = CN(TN,$1); ALL($$,$2,$4); }
;

loop_label : T_WORD  { $$ = CT(TN,LN); }
;

output : VISIBLE expr     { $$ = CT(TN,LN); AL($$,$2); }
       | VISIBLE expr P_EXCL     { $$ = CT(TN,LN); AL($$,$2); }
;

prog_start : HAI end_stmt { $$ = $1 }
;

prog_end   : KTHXBYE { $$ = $1 }
           | prog_end end_stmt { $$ = $1 }
;

self_assignment : UPZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN,LN); ALL($$,$2,$5); }
                | NERFZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN,LN); ALL($$,$2,$5); }
                | TIEMZD l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN,LN); ALL($$,$2,$5); }
                | OVARZ l_value P_EXCL P_EXCL increment_expr     { $$ = CN(TN,LN); ALL($$,$2,$5); }
;

stmt : include               { $$ = CN(TN,LN); AL($$,$1); }
     | declaration           { $$ = CN(TN,LN); AL($$,$1); }
     | loop                  { $$ = CN(TN,LN); AL($$,$1); }
     | conditional           { $$ = CN(TN,LN); AL($$,$1); }
     | assignment            { $$ = CN(TN,LN); AL($$,$1); }
     | input                 { $$ = CN(TN,LN); AL($$,$1); }
     | output                { $$ = CN(TN,LN); AL($$,$1); }
     | expr                  { $$ = CN(TN,LN); AL($$,$1); }
     | GTFO                  { $$ = CT(TN,LN); }
     | exit                  { $$ = CN(TN,LN); AL($$,$1); }
     | COMMENT               { $$ = CT(TN,LN); }
;

stmts : /* No statements at all */     { $$ = CT(TN,LN); }
      | stmts end_stmt /* empty line */     { $$ = CN(TN,LN); AL($$,$1); }
      | stmt end_stmt            { $$ = CN(TN,LN); AL($$,$1); }
      | stmts stmt end_stmt      { $$ = CN(TN,LN); ALL($$,$2,$1); }
;

then : end_stmt
     | P_QMARK end_stmt
     | end_stmt YARLY
     | P_QMARK end_stmt YARLY
;

%%
void yyerror(const char *str)
{
  fprintf(stderr,"error: %s on line %ld\n",str,lineno);
}
 
int yywrap()
{
  return 1;
}

ast_node *generate_ast()
{
  type_names = yytname;
  type_count = YYNRULES;

  root = NULL;

  yyparse();

  return root;
} 
