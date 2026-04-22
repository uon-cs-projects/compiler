#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "shared_types.h"
#include "grammar.h"
#include "token_stream.h"

extern int error_count;

/* Use the pointer to the typedef Token */
void error_match(int expected_tok_type, const Token *got);
void error_predict(int nonterminal_idx, const Token *got);

/* Note: ensure struct TokenStream is defined in shared_types.h or similar */
Token *error_recover(int nonterminal_idx, TokenStream *ts);

#endif