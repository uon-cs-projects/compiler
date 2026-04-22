/* lexer_adapter.h */
#ifndef LEXER_ADAPTER_H
#define LEXER_ADAPTER_H

#include "src/parser/shared_types.h"

/*
 * Call the existing lexer on 'source_code', then convert its output
 * into the parser's Token array format.
 * 'out_tokens' must be allocated by the caller (size MAX_TOKENS).
 * Returns the number of tokens written (including the appended EOF).
 */
int lexer_adapter_run(const char *source_code,
                      Token      *out_tokens,
                      int         max_tokens);

#endif /* LEXER_ADAPTER_H */
