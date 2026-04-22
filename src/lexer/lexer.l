%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokens.h"
#include "symbol_table.h"   /* Added: For insert and current_scope */
#include "error_handler.h"  /* Added: For centralized error reporting */

const char* get_token_name(int token);
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
"def"           { return T_DEF; }
"else if"       { return T_ELSEIF; }
"else"          { return T_ELSE; }
"if"            { return T_IF; }
"while"         { return T_WHILE; }
"for"           { return T_FOR; }
"return"        { return T_RETURN; }
"print"         { return T_PRINT; }
"break"         { return T_BREAK; }
"and"           { return T_AND; }
"or"            { return T_OR; }
"not"           { return T_NOT; }

    /* 2. Literals */
"True"          { return T_BOOL; }
"False"         { return T_BOOL; }

{DIGIT}+{LETTER}+{ID}* { 
    char msg[100];
    snprintf(msg, sizeof(msg), "Invalid identifier '%s' - identifiers cannot start with a digit", yytext);
    report_error(yylineno, "LEXICAL", msg); 
    
    /* We return T_ID anyway so the Parser doesn't immediately crash. 
       This allows the compiler to find more errors later in the file. */
    return T_ID; 
}

{ID} { 
    /* 1. Look up the name first */
    Symbol *s = lookup(yytext);

    /* 2. Only call insert if it DOESN'T exist yet */
    if (s == NULL) {
        insert(yytext, T_ID, yylineno, current_scope); 
    }
    
    return T_ID; 
}
{DIGIT}+        { return T_INT; }

\"[^\"\n]*\"    { return T_STR; }

    /* 3. Operators & Delimiters */
"=="            { return T_EQ; }
"!="            { return T_NEQ; }
">="            { return T_GE; }
"<="            { return T_LE; }
">"             { return T_GT; }
"<"             { return T_LT; }
"="             { return T_ASSIGN; }
"+"             { return T_PLUS; }
"-"             { return T_MINUS; }
"*"             { return T_MULT; }
"/"             { return T_DIV; }
"("             { return T_LPAREN; }
")"             { return T_RPAREN; }
"{"             { return T_LBRACE; }
"}"             { return T_RBRACE; }
"["             { return T_LBRACKET; }
"]"             { return T_RBRACKET; }
","             { return T_COMMA; }

{WS}            { /* Skip */ }
{NEWLINE}       { /* Skip */ }

.               { 
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