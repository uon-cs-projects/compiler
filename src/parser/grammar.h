#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "shared_types.h"

#define MAX_NONTERMINALS   20
#define MAX_TERMINALS      TOKEN_TYPE_COUNT
#define MAX_PRODUCTIONS    50
#define MAX_RHS_LENGTH     10

/* Nonterminal index constants */
typedef enum {
    NT_PROGRAM = 0,
    NT_STMT_LIST,
    NT_STMT,
    NT_BLOCK,
    NT_EXPR,
    NT_EXPR_TAIL,
    NT_TERM,
    NT_TERM_TAIL,
    NT_FACTOR,
    NONTERMINAL_COUNT   /* always last */
} NonterminalIndex;

extern const char *NONTERMINAL_NAMES[NONTERMINAL_COUNT];

/* One grammar production: LHS -> RHS[0] RHS[1] ... RHS[len-1]  */
/* len == 0 means ε (empty production)                          */
typedef struct {
    NonterminalIndex lhs;
    Symbol           rhs[MAX_RHS_LENGTH];
    int              len;     /* number of symbols in RHS */
} Production;

extern Production PRODUCTIONS[MAX_PRODUCTIONS];
extern int        NUM_PRODUCTIONS;

void grammar_init(void);   /* populate PRODUCTIONS[] */

#endif /* GRAMMAR_H */