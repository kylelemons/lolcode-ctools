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

// Duplicate a char for inclusion in the parse tree
void *cdup(char x)
{
  char *r = (char *)malloc(sizeof(char));
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
%token <ulong> ARGSEP AND GREATER BYES INCLUDE COMMENT DIAF FAIL GIMMEH GTFO HAI
%token <ulong> DECLARE INFLOOP ITZ IZ KTHX KTHXBYE EQUALTO LETTAR LINE LOL
%token <ulong> MINUS MINUSEQ NOT NOWAI OR OUTTA DIV DIVEQ R 
%token <ulong> STDIN MULT MULTEQ PLUS PLUSEQ PRINT WIN
%token <ulong> WORD XOR YARLY P_QMARK

%left PLUSEQ MINUSEQ MULTEQ DIVEQ R
%left NOT
%left AND OR XOR
%left GREATER LESS EQUALTO
%left PLUS MINUS
%left MULT DIV
%left ARR

%type <node> word string number

%type <node> array assignment brk comment condexpr conditional
%type <node> declaration exit exit_status exit_message expr 
%type <node> include increment_expr initializer input_type input_from input 
%type <node> loop output program
%type <node> self_assignment stmt stmts
%type <ulong> prog_start prog_end

%expect 94

%start program

%%
program : prog_start stmts prog_end { $$ = CN(TN,$1); AL($$,$2); root = $$; }
;

argsep : ARGSEP
       | /* nothin */
;

array : ARR word P_EXCL P_EXCL expr { $$ = CN(TN,LN); ALL($$,$2,$5); } 
      | word                        { $$ = CN(TN,LN); AL($$,$1); }
;

/*TODO: Figure out how to allow an expr as the index... */
/*
array_index : expr ARR        { $$ = CN(TN,LN); AL($$,$1); }
            | array_index ARR { $$ = CN(TN,LN); AL($$,$1); }
;
*/

/*
array_index : T_NUMBER ARR { $$ = CT(TN,LN); AL($$,idup($1)); }
            | T_WORD ARR { $$ = CT(TN,LN); AL($$,$1); }
;
*/

assignment : LOL array R expr     { $$ = CN(TN,LN); ALL($$,$2,$4); }
;

brk : GTFO { $$ = CT(TN,LN); }
;

comment : COMMENT { $$ = CT(TN,LN); }
;

condexpr : WIN                      { int b = 1; $$ = CT(TN,LN); AL($$,idup(b)); }
         | FAIL                     { int b = 0; $$ = CT(TN,LN); AL($$,idup(b)); }
         | GREATER expr argsep expr        { $$ = CN(TN,LN); ALLL($$,cdup('>'),$2,$4); }
         | LESS expr argsep expr           { $$ = CN(TN,LN); ALLL($$,cdup('<'),$2,$4); }
         | EQUALTO expr argsep expr        { $$ = CN(TN,LN); ALLL($$,cdup('='),$2,$4); }
         | OR condexpr argsep condexpr     { $$ = CN(TN,LN); ALLL($$,cdup('|'),$2,$4); }
         | AND condexpr argsep condexpr    { $$ = CN(TN,LN); ALLL($$,cdup('&'),$2,$4); }
         | XOR condexpr argsep condexpr    { $$ = CN(TN,LN); ALLL($$,cdup('^'),$2,$4); }
         | NOT condexpr                    { $$ = CN(TN,LN); ALL($$,cdup('!'),$2); }
;

conditional : IZ condexpr then stmts KTHX     { $$ = CN(TN,$1); ALL($$,$2,$4); }
            | IZ condexpr then stmts elsethen stmts KTHX     { $$ = CN(TN,$1); ALLL($$,$2,$4,$6); }
;

declaration : DECLARE array initializer     { $$ = CN(TN,LN); ALL($$,$2,$3); }
;

elsethen : NOWAI end_stmt
;

end_stmt : NEWLINE           { lineno = $1; }
;

exit : DIAF exit_status exit_message     { $$ = CN(TN,LN); ALL($$,$2,$3); }
     | BYES exit_status exit_message     { $$ = CN(TN,LN); ALL($$,$2,$3); }
;

exit_status : /* nothing */ { $$ = CT(TN,LN); }
            | number        { $$ = CT(TN,LN); AL($$,$1); }
            | word          { $$ = CT(TN,LN); AL($$,$1); }
;

exit_message : /* nothing */ { $$ = CT(TN,LN); }
             | string        { $$ = CT(TN,LN); AL($$,$1); }
             | word          { $$ = CT(TN,LN); AL($$,$1); }
;

/*
array_expr : array              { $$ = CN(TN,LN); AL($$,$1); }
           | array PLUS expr      { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array MINUS expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array MULT expr   { $$ = CN(TN,LN); ALL($$,$1,$3); }
           | array DIV expr    { $$ = CN(TN,LN); ALL($$,$1,$3); }
;
*/
/*
index_expr : word                          { $$ = CN(TN,LN); AL($$,$1); }
           | number                        { $$ = CN(TN,LN); AL($$,$1); }
           | index_expr PLUS index_expr      { $$ = CN(TN,LN); ALLL($$,$1,cdup('+'),$3); }
           | index_expr MINUS index_expr    { $$ = CN(TN,LN); ALLL($$,$1,cdup('-'),$3); }
           | index_expr MULT index_expr   { $$ = CN(TN,LN); ALLL($$,$1,cdup('*'),$3); }
           | index_expr DIV index_expr    { $$ = CN(TN,LN); ALLL($$,$1,cdup('/'),$3); }
;
*/

expr : string            { $$ = $1; }
     | number            { $$ = $1; }
     | array             { $$ = $1; }
     | PLUS expr argsep expr    { $$ = CN(TN,LN); ALLL($$,cdup('+'),$2,$4); }
     | MINUS expr argsep expr   { $$ = CN(TN,LN); ALLL($$,cdup('-'),$2,$4); }
     | MULT expr argsep expr    { $$ = CN(TN,LN); ALLL($$,cdup('*'),$2,$4); }
     | DIV expr argsep expr     { $$ = CN(TN,LN); ALLL($$,cdup('/'),$2,$4); }
;

include : INCLUDE word P_QMARK   { $$ = CN(TN,LN); AL($$,$2); }
        | INCLUDE string P_QMARK { $$ = CN(TN,LN); AL($$,$2); }

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

input_from : /* empty */     { $$ = CT(TN,LN); }
           | OUTTA word      { $$ = $2; }
           | OUTTA STDIN     { $$ = CT(TN,LN); }
;

input : GIMMEH input_type array input_from { $$ = CN(TN,LN); ALLL($$,$2,$3,$4); }
;

loop : INFLOOP word end_stmt stmts KTHX     { $$ = CN(TN,$1); ALL($$,$2,$4); }
;

number : T_NUMBER         { $$ = CT(TN,LN); AL($$,idup($1)); }

output : PRINT expr     { $$ = CT(TN,LN); AL($$,$2); }
       | PRINT expr P_EXCL     { $$ = CT(TN,LN); ALL($$,$2,cdup('!')); }
;

prog_start : HAI end_stmt { $$ = $1 }
;

prog_end   : KTHXBYE { $$ = $1 }
           | prog_end end_stmt { $$ = $1 }
;

self_assignment : PLUSEQ array P_EXCL P_EXCL increment_expr    { $$ = CN(TN,LN); ALLL($$,cdup('+'),$2,$5); }
                | MINUSEQ array P_EXCL P_EXCL increment_expr   { $$ = CN(TN,LN); ALLL($$,cdup('-'),$2,$5); }
                | MULTEQ array P_EXCL P_EXCL increment_expr    { $$ = CN(TN,LN); ALLL($$,cdup('*'),$2,$5); }
                | DIVEQ array P_EXCL P_EXCL increment_expr     { $$ = CN(TN,LN); ALLL($$,cdup('/'),$2,$5); }
;

stmt : include               { $$ = $1; }
     | declaration           { $$ = $1; }
     | loop                  { $$ = $1; }
     | conditional           { $$ = $1; }
     | assignment            { $$ = $1; }
     | self_assignment       { $$ = $1; }
     | input                 { $$ = $1; }
     | output                { $$ = $1; }
     | expr                  { $$ = $1; }
     | brk                   { $$ = $1; }
     | exit                  { $$ = $1; }
     | comment               { $$ = $1; }
;

stmts : /* No statements at all */     { $$ = CT(TN,LN); }
      | stmts end_stmt /* empty line */     { $$ = CN(TN,LN); AL($$,$1); }
      | stmt end_stmt            { $$ = CN(TN,LN); AL($$,$1); }
      | stmts stmt end_stmt      { $$ = CN(TN,LN); ALL($$,$1,$2); }
;

string : T_STRING { $$ = CT(TN,LN); AL($$,$1); }

then : end_stmt
     | P_QMARK end_stmt
     | end_stmt YARLY
     | P_QMARK end_stmt YARLY
;

word : T_WORD    { $$ = CT(TN,LN); AL($$,$1); }

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
  type_count = YYNTOKENS+YYNNTS;

  root = NULL;

  yyparse();

  return root;
} 
