#ifndef TOKENS_H
#define TOKENS_H

/* --- Keywords --- */
#define T_DEF       101  /* Keyword: 'def' (function definition) */
#define T_IF        102  /* Keyword: 'if' (conditional start) */
#define T_ELSE      103  /* Keyword: 'else' (fallback condition) */
#define T_ELSEIF    104  /* Keyword: 'else if' (alternative condition) */
#define T_WHILE     105  /* Keyword: 'while' (looping) */
#define T_FOR       106  /* Keyword: 'for' (looping) */
#define T_RETURN    107  /* Keyword: 'return' (function exit) */
#define T_PRINT     108  /* Keyword: 'print' (output to console) */
#define T_BREAK     109  /* Keyword: 'break' (exit loop) */

/* --- Logical Operators --- */
#define T_AND       201  /* Logical Operator: 'and' */
#define T_OR        202  /* Logical Operator: 'or' */
#define T_NOT       203  /* Logical Operator: 'not' */

/* --- Literals & Identifiers --- */
#define T_ID        301  /* Variable/Function Name (e.g., 'result', '_temp') */
#define T_INT       302  /* Integer Literal (e.g., 20, 100) */
#define T_STR       303  /* String Literal (e.g., "Hello World") */
#define T_BOOL      304  /* Boolean Literal (True or False) */

/* --- Math Operators --- */
#define T_PLUS      401  /* Arithmetic: '+' */
#define T_MINUS     402  /* Arithmetic: '-' */
#define T_MULT      403  /* Arithmetic: '*' */
#define T_DIV       404  /* Arithmetic: '/' */

/* --- Comparison & Assignment --- */
#define T_EQ        501  /* Comparison: '==' (equals) */
#define T_NEQ       502  /* Comparison: '!=' (not equals) */
#define T_GT        503  /* Comparison: '>'  (greater than) */
#define T_LT        504  /* Comparison: '<'  (less than) */
#define T_GE        505  /* Comparison: '>=' (greater or equal) */
#define T_LE        506  /* Comparison: '<=' (less or equal) */
#define T_ASSIGN    507  /* Assignment Operator: '=' */

/* --- Delimiters --- */
#define T_LPAREN    601  /* Delimiter: '(' (left parenthesis) */
#define T_RPAREN    602  /* Delimiter: ')' (right parenthesis) */
#define T_LBRACE    603  /* Delimiter: '{' (left curly brace) */
#define T_RBRACE    604  /* Delimiter: '}' (right curly brace) */
#define T_COMMA     605  /* Delimiter: ',' (comma separator) */
#define T_LBRACKET  606  /* Delimiter: '[' (left square bracket) */
#define T_RBRACKET  607  /* Delimiter: ']' (right square bracket) */

#endif