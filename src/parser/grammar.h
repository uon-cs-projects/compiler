#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "shared_types.h"

#define MAX_NONTERMINALS   40
#define MAX_TERMINALS      TOKEN_TYPE_COUNT
#define MAX_PRODUCTIONS    100
#define MAX_RHS_LENGTH     10

typedef enum {
    NT_PROGRAM = 0,
    NT_PROGRAM_TAIL,
    NT_ELEMENT,
    NT_FUNCTION,
    NT_PARAM_LIST,
    NT_PARAM_LIST_TAIL,
    NT_BLOCK,
    NT_STMT_LIST,
    NT_STMT,
    NT_STMT_ID_TAIL,
    NT_IF_STMT,
    NT_IF_TAIL,
    NT_WHILE_STMT,
    NT_FOR_STMT,
    NT_PRINT_STMT,
    NT_BREAK_STMT,
    NT_EXPR,
    NT_EXPR_TAIL,
    NT_AND_EXPR,
    NT_AND_EXPR_TAIL,
    NT_NOT_EXPR,
    NT_COMP_EXPR,
    NT_COMP_EXPR_TAIL,
    NT_MATH_EXPR,
    NT_MATH_EXPR_TAIL,
    NT_TERM,
    NT_TERM_TAIL,
    NT_FACTOR,
    NT_FACTOR_ID_TAIL,
    NT_ARG_LIST,
    NT_ARG_LIST_TAIL,
    NONTERMINAL_COUNT   
} NonterminalIndex;

extern const char *NONTERMINAL_NAMES[NONTERMINAL_COUNT];

typedef struct {
    NonterminalIndex lhs;
    GrammarSymbol           rhs[MAX_RHS_LENGTH];
    int              len;     
} Production;

extern Production PRODUCTIONS[MAX_PRODUCTIONS];
extern int        NUM_PRODUCTIONS;

void grammar_init(void);

#endif /* GRAMMAR_H */