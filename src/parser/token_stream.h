#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include "grammar.h"   /* pulls in shared_types.h -> Token, TokenType */

#define MAX_TOKENS 1024

typedef struct {
    Token tokens[MAX_TOKENS];
    int   count;
    int   pos;        /* current read position */
} TokenStream;

void   ts_init   (TokenStream *ts, Token *arr, int count);
Token *ts_peek   (TokenStream *ts);   /* lookahead — does NOT advance */
Token  ts_consume(TokenStream *ts);   /* return current token, advance */

#endif /* TOKEN_STREAM_H */