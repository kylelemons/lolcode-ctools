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
BTW.*$                  { return COMMENT; }
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

"AND"                    { return AND; }
"BIGR THAN"              { return BIGR_THAN; }
"BYES"                   { return BYES; }
"CAN HAS"                { return CAN_HAS; }
"DIAF"                   { return DIAF; }
"GIMMEH"                 { return GIMMEH; }
"GTFO"                   { return GTFO; }
"HAI"                    { return HAI; }
"I HAS A"                { return I_HAS_A; }
"IM IN YR"               { return IM_IN_YR; }
"IN MAH"                 { return IN_MAH; }
"ITZ"                    { return ITZ; }
"IZ"                     { return IZ; }
"KTHX"                   { return KTHX; }
"KTHXBYE"                { return KTHXBYE; }
"LETTAR"                 { return LETTAR; }
"LIEK"                   { return LIEK; }
"LINE"                   { return LINE; }
"LOL"                    { return LOL; }
"NERF"                   { return NERF; }
"NERFZ"                  { return NERFZ; }
"NOT"                    { return NOT; }
"NOWAI"                  { return NOWAI; }
"OR"                     { return OR; }
"OUTTA"                  { return OUTTA; }
"OVAR"                   { return OVAR; }
"OVARZ"                  { return OVARZ; }
"R"                      { return R; }
"SMALR THAN"             { return SMALR_THAN; }
"STDIN"                  { return STDIN; }
"TIEMZ"                  { return TIEMZ; }
"TIEMZD"                 { return TIEMZD; }
"UP"                     { return UP; }
"UPZ"                    { return UPZ; }
"VISIBLE"                { return VISIBLE; }
"WORD"                   { return WORD; }
"XOR"                    { return XOR; }
"YARLY"                  { return YARLY; }
\?                      { return P_QMARK; }
\!                      { return P_EXCL; }
[A-Za-z]+               { yylval.str = strdup(yytext); return T_WORD; }
[.\n]                   { if (*yytext == '\n') ++curline; yylval.ulong=curline; return NEWLINE; }
[\t ]+                  { /* Ignore whitespace */; }
%%
