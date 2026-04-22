#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include "lexer/tokens.h"

/* Max token ID in tokens.h is 607 (T_RBRACKET). 700 provides a safe upper bound. */
#define TOKEN_TYPE_COUNT 700 
#define TOK_EOF 0

typedef enum {
    SYM_TERMINAL,
    SYM_NONTERMINAL,
    SYM_EPSILON
} SymbolKind;

typedef struct {
    SymbolKind kind;
    int index; /* Holds Token ID for terminals, or NonterminalIndex for nonterminals */
} GrammarSymbol;

typedef struct {
    int id;
    char *lexeme;
    int line;
    int col;
} Token;

#endif /* SHARED_TYPES_H */