#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "../parser/shared_types.h"
#include "../parser/token_stream.h"
#include "../parser/grammar.h"

extern int error_count;
extern int warning_count;

/* Standardized reporting functions */
void report_error(int line, const char *phase, const char *message);
void report_warning(int line, const char *phase, const char *message);

/* Use the pointer to the typedef Token */
void error_match(int expected_tok_type, const Token *got);
void error_predict(int nonterminal_idx, const Token *got);

/* Note: ensure struct TokenStream is defined in shared_types.h or similar */
Token *error_recover(int nonterminal_idx, TokenStream *ts);


/* The final "Dashboard" summary */
void finalize_compilation();

#endif