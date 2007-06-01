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
#define TN yyr1[yyn]
ast_node *root = NULL;

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
}

%token <num> T_NUMBER 
%token <str> T_WORD T_STRING
%token <num> P_EXCL NEWLINE
%token <num> AND BIGR_THAN BYES CAN_HAS COMMENT DIAF GIMMEH GTFO HAI
%token <num> I_HAS_A IM_IN_YR ITZ IZ KTHX KTHXBYE LIEK LETTAR LINE LOL
%token <num> NERF NERFZ NOT NOWAI OR OUTTA OVAR OVARZ R 
%token <num> STDIN TIEMZ TIEMZD UP UPZ VISIBLE 
%token <num> WORD XOR YARLY P_QMARK

%left UPZ NERFZ TIEMZD OVARZ R
%left NOT
%left AND OR XOR
%left BIGR_THAN SMALR_THAN LIEK
%left UP NERF
%left TIEMZ OVAR
%left IN_MAH

%type <node> array array_index assignment condexpr conditional
%type <node> declaration exit exit_status exit_message expr 
%type <node> include increment_expr initializer input_type input_from input 
%type <node> l_value loop loop_label output program
%type <node> self_assignment stmt stmts

%expect 74

%start program

%%
program : prog_start stmts prog_end { printf("Valid program!\n"); $$ = CN(TN); AL($$,$2); root = $$; }
;

array : array_index array  { $$ = CN(TN); ALL($$,$1,$2); } 
      | T_WORD             { printf("[VAR:%s]", $1); $$ = CT(TN); AL($$,$1); }
;

/*TODO: Figure out how to allow an expr as the index... */
/*
array_index : expr IN_MAH  { $$ = CN(TN); AL($$,$1); }
*/
array_index : T_NUMBER IN_MAH { $$ = CT(TN); AL($$,idup($1)); }
            | T_WORD IN_MAH { $$ = CT(TN); AL($$,$1); }
;

assignment : LOL l_value R expr     { $$ = CN(TN); ALL($$,$2,$4); }
           | self_assignment      { $$ = CN(TN); }
;

condexpr : expr BIGR_THAN expr      { printf("[>]"); $$ = CN(TN); ALL($$,$1,$3); }
         | expr NOT BIGR_THAN expr  { printf("[<=]"); $$ = CN(TN); ALL($$,$1,$4); }
         | expr SMALR_THAN expr     { printf("[<]"); $$ = CN(TN); ALL($$,$1,$3); }
         | expr NOT SMALR_THAN expr { printf("[>=]");  $$ = CN(TN); ALL($$,$1,$4); }
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

declaration : I_HAS_A array initializer     { $$ = CN(TN); ALL($$,$2,$3); }
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

include : CAN_HAS T_WORD P_QMARK   { $$ = CT(TN); AL($$,$2); }
        | CAN_HAS T_STRING P_QMARK { $$ = CT(TN); AL($$,$2); }

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

loop : IM_IN_YR loop_label end_stmt stmts KTHX     { $$ = CN(TN); ALL($$,$2,$4); }
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

ast_node *generate_ast()
{
  type_names = yytname;

  root = NULL;

  yyparse();

  return root;
} 
