%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "grammar.tab.h"

unsigned long curline = 1;

char *string_buf = NULL;
unsigned long strsize = 0;

void str_err(const char *err)
{
  fprintf(stderr, "error in lex (line %d col %d): %s\n", -1 /*yyline*/, -1/*yycolumn*/, err);
  exit(1);
}

void str_app(char ch)
{
  string_buf = (char *)realloc(string_buf, (strsize+1) * sizeof(char));
  string_buf[strsize++] = ch;
}

%}

%x strstate

%%
BTW.*$                  { yylval.ulong=curline; return COMMENT; }
[0-9]+                  { yylval.num = atoi(yytext); return T_NUMBER; }
\"                      { string_buf=NULL; strsize=0; BEGIN(strstate); }

<strstate>{
  \"                { /* saw closing quote - all done */
                      BEGIN(INITIAL);
                      str_app('\0'); // null terminate
                      /* return string constant token type and
                       * value to parser
                       */
                      yylval.str = string_buf;
                      return T_STRING;
                    }

  \n                {
                      ++curline;
                      /* error - unterminated string constant */
                      /* generate error message */
                      str_err("Unterminated string constant - \\ needed?");
                    }

  \\[0-7]{1,3}      {
                      /* octal escape sequence */
                      int result;

                      (void) sscanf( yytext + 1, "%o", &result );

                      if ( result > 0xff )
                              /* error, constant is out-of-bounds */

                      str_app((char)result);
                    }

  \\[0-9]+          {
                      /* generate error - bad escape sequence; something
                       * like '\48' or '\0777777'
                       */
                      str_err("Bad escape sequence");
                    }

  \\n               str_app('\n');
  \\t               str_app('\t');
  \\r               str_app('\r');
  \\b               str_app('\b');
  \\f               str_app('\f');

  \\(.|\n)          str_app(yytext[1]);

  [^\\\n\"]+        {
                      char *yptr = yytext;

                      while ( *yptr )
                        str_app(*yptr++);
                    }
}

"AN"                     { yylval.ulong=curline; return ARGSEP; }
"AND"                    { yylval.ulong=curline; return AND; }
"BIGR"                   { yylval.ulong=curline; return GREATER; }
"BYES"                   { yylval.ulong=curline; return BYES; }
"CAN HAS"                { yylval.ulong=curline; return INCLUDE; }
"DEN"                    { yylval.ulong=curline; return ARGSEP; }
"DIAF"                   { yylval.ulong=curline; return DIAF; }
"FAIL"                   { yylval.ulong=curline; return FAIL; }
"GIMMEH"                 { yylval.ulong=curline; return GIMMEH; }
"GTFO"                   { yylval.ulong=curline; return GTFO; }
"HAI"                    { yylval.ulong=curline; return HAI; }
"I HAS A"                { yylval.ulong=curline; return DECLARE; }
"IM IN YR"               { yylval.ulong=curline; return INFLOOP; }
"MAH"                    { yylval.ulong=curline; return ARR; }
"ITZ"                    { yylval.ulong=curline; return ITZ; }
"IZ"                     { yylval.ulong=curline; return IZ; }
"KTHX"                   { yylval.ulong=curline; return KTHX; }
"KTHXBYE"                { yylval.ulong=curline; return KTHXBYE; }
"LETTAR"                 { yylval.ulong=curline; return LETTAR; }
"LIEK"                   { yylval.ulong=curline; return EQUALTO; }
"LINE"                   { yylval.ulong=curline; return LINE; }
"LOL"                    { yylval.ulong=curline; return LOL; }
LOL(OL)+                 { yylval.ulong=curline; yyless(2); unput('\n'); curline--; return LOL; }
"NERF"                   { yylval.ulong=curline; return MINUS; }
"NERFZ"                  { yylval.ulong=curline; return MINUSEQ; }
"NOT"                    { yylval.ulong=curline; return NOT; }
"NOWAI"                  { yylval.ulong=curline; return NOWAI; }
"OR"                     { yylval.ulong=curline; return OR; }
"OUTTA"                  { yylval.ulong=curline; return OUTTA; }
"OVAR"                   { yylval.ulong=curline; return DIV; }
"OVARZ"                  { yylval.ulong=curline; return DIVEQ; }
"R"                      { yylval.ulong=curline; return R; }
"SMALR"                  { yylval.ulong=curline; return INCLUDE; }
"STDIN"                  { yylval.ulong=curline; return STDIN; }
"TIEMZ"                  { yylval.ulong=curline; return MULT; }
"TIEMZD"                 { yylval.ulong=curline; return MULTEQ; }
"UP"                     { yylval.ulong=curline; return PLUS; }
"UPZ"                    { yylval.ulong=curline; return PLUSEQ; }
"VISIBLE"                { yylval.ulong=curline; return PRINT; }
"WIN"                    { yylval.ulong=curline; return WIN; }
"WORD"                   { yylval.ulong=curline; return WORD; }
"XOR"                    { yylval.ulong=curline; return XOR; }
"YARLY"                  { yylval.ulong=curline; return YARLY; }
\?                      { yylval.ulong=curline; return P_QMARK; }
\!                      { yylval.ulong=curline; return P_EXCL; }
[A-Za-z0-9_]+           { yylval.str = strdup(yytext); return T_WORD; }
[.\n]                   { if (*yytext == '\n') ++curline; yylval.ulong=curline; return NEWLINE; }
[\t ]+                  { /* Ignore whitespace */; }
%%
