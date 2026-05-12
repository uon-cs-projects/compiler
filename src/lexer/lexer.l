%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokens.h"
#include "error_handler.h"  /* Added: For centralized error reporting */

const char* get_token_name(int token);
int yycolumn = 1;
%}

%option yylineno
%option noyywrap

DIGIT    [0-9]
LETTER   [a-zA-Z]
ID       [a-zA-Z_][a-zA-Z0-9_]*
WS       [ \t\r]+
NEWLINE  \n

%%

    /* 1. Keywords */
"def"           { yycolumn += yyleng; return T_DEF; }
"else if"       { yycolumn += yyleng; return T_ELSEIF; }
"else"          { yycolumn += yyleng; return T_ELSE; }
"if"            { yycolumn += yyleng; return T_IF; }
"while"         { yycolumn += yyleng; return T_WHILE; }
"for"           { yycolumn += yyleng; return T_FOR; }
"to"            { yycolumn += yyleng; return T_TO; }
"return"        { yycolumn += yyleng; return T_RETURN; }
"print"         { yycolumn += yyleng; return T_PRINT; }
"break"         { yycolumn += yyleng; return T_BREAK; }
"and"           { yycolumn += yyleng; return T_AND; }
"or"            { yycolumn += yyleng; return T_OR; }
"not"           { yycolumn += yyleng; return T_NOT; }

    /* 2. Literals */
"True"          { yycolumn += yyleng; return T_BOOL; }
"False"         { yycolumn += yyleng; return T_BOOL; }

{DIGIT}+{LETTER}+{ID}* { 
    yycolumn += yyleng;
    char msg[100];
    snprintf(msg, sizeof(msg), "Invalid identifier '%s' - identifiers cannot start with a digit", yytext);
    report_error(yylineno, "LEXICAL", msg); 
    
    /* We return T_ID anyway so the Parser doesn't immediately crash. 
       This allows the compiler to find more errors later in the file. */
    return T_ID; 
}

{ID} { 
    yycolumn += yyleng;
    return T_ID; 
}
{DIGIT}+        { yycolumn += yyleng; return T_INT; }

\"[^\"\n]*\"    { yycolumn += yyleng; return T_STR; }

    /* 3. Operators & Delimiters */
"=="            { yycolumn += yyleng; return T_EQ; }
"!="            { yycolumn += yyleng; return T_NEQ; }
">="            { yycolumn += yyleng; return T_GE; }
"<="            { yycolumn += yyleng; return T_LE; }
">"             { yycolumn += yyleng; return T_GT; }
"<"             { yycolumn += yyleng; return T_LT; }
"="             { yycolumn += yyleng; return T_ASSIGN; }
"+"             { yycolumn += yyleng; return T_PLUS; }
"-"             { yycolumn += yyleng; return T_MINUS; }
"*"             { yycolumn += yyleng; return T_MULT; }
"/"             { yycolumn += yyleng; return T_DIV; }
"("             { yycolumn += yyleng; return T_LPAREN; }
")"             { yycolumn += yyleng; return T_RPAREN; }
"{"             { yycolumn += yyleng; return T_LBRACE; }
"}"             { yycolumn += yyleng; return T_RBRACE; }
"["             { yycolumn += yyleng; return T_LBRACKET; }
"]"             { yycolumn += yyleng; return T_RBRACKET; }
","             { yycolumn += yyleng; return T_COMMA; }

{WS}            { yycolumn += yyleng; /* Skip */ }
{NEWLINE}       { yycolumn = 1; /* Skip */ }

.               { 
                  yycolumn += yyleng;
                  /* LOGIC: Use the standardized error handler */
                  char msg[50];
                  sprintf(msg, "Unexpected character '%s'", yytext);
                  report_error(yylineno, "LEXICAL", msg); 
                }

%%



#ifdef STANDALONE_LEXER
int main(int argc, char **argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }

    printf("%-12s\t%-10s\t%-15s\t%-5s\n", "TOKEN NAME", "ID", "LEXEME", "LINE");
    printf("------------------------------------------------------------\n");

    int token;
    while ((token = yylex())) {
        printf("%-12s\t%-10d\t%-15s\t%-5d\n", 
               get_token_name(token), token, yytext, yylineno);
    }

    /* 1. Print the discovered identifiers */
    print_symbol_table();

    /* 2. Print the summary of errors/warnings */
    finalize_compilation();

    return 0;
}
#endif