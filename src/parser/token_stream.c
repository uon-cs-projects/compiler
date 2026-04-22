#include "token_stream.h"
#include <string.h>

/* Returned when the stream is exhausted */
static Token EOF_TOKEN = { TOK_EOF, "$", 0, 0 };

void ts_init(TokenStream *ts, Token *arr, int count) {
    /* Copy the token array; guard against missing EOF */
    int copy = (count < MAX_TOKENS) ? count : MAX_TOKENS - 1;
    memcpy(ts->tokens, arr, copy * sizeof(Token));
    ts->count = copy;
    ts->pos   = 0;

    /* Ensure the last token is always EOF */
    if (copy == 0 || ts->tokens[copy - 1].id != TOK_EOF) {
        ts->tokens[copy] = EOF_TOKEN;
        ts->count++;
    }
}

Token *ts_peek(TokenStream *ts) {
    if (ts->pos >= ts->count) return &EOF_TOKEN;
    return &ts->tokens[ts->pos];
}

Token ts_consume(TokenStream *ts) {
    if (ts->pos >= ts->count) return EOF_TOKEN;
    return ts->tokens[ts->pos++];
}