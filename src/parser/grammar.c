#include "grammar.h"

const char *NONTERMINAL_NAMES[NONTERMINAL_COUNT] = {
    "PROGRAM", "STMT_LIST", "STMT", "BLOCK",
    "EXPR", "EXPR_TAIL", "TERM", "TERM_TAIL", "FACTOR"
};

Production PRODUCTIONS[MAX_PRODUCTIONS];
int        NUM_PRODUCTIONS = 0;

/* Helper macros to build a Symbol */
#define T(tok)  ((Symbol){ SYM_TERMINAL,    (tok)  })
#define N(nt)   ((Symbol){ SYM_NONTERMINAL, (nt)   })
#define EPS     ((Symbol){ SYM_EPSILON,     0      })

void grammar_init(void) {
    int i = 0;

    /* 0: PROGRAM -> STMT_LIST */
    PRODUCTIONS[i].lhs = NT_PROGRAM;
    PRODUCTIONS[i].rhs[0] = N(NT_STMT_LIST);
    PRODUCTIONS[i].len = 1; i++;

    /* 1: STMT_LIST -> STMT STMT_LIST */
    PRODUCTIONS[i].lhs = NT_STMT_LIST;
    PRODUCTIONS[i].rhs[0] = N(NT_STMT);
    PRODUCTIONS[i].rhs[1] = N(NT_STMT_LIST);
    PRODUCTIONS[i].len = 2; i++;

    /* 2: STMT_LIST -> ε */
    PRODUCTIONS[i].lhs = NT_STMT_LIST;
    PRODUCTIONS[i].len = 0; i++;

    /* 3: STMT -> BLOCK */
    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = N(NT_BLOCK);
    PRODUCTIONS[i].len = 1; i++;

    /* 4: STMT -> print ( EXPR ) */
    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = T(T_PRINT);
    PRODUCTIONS[i].rhs[1] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[2] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[3] = T(T_RPAREN);
    PRODUCTIONS[i].len = 4; i++;

    /* 5: STMT -> id = EXPR */
    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = T(T_ID);
    PRODUCTIONS[i].rhs[1] = T(T_ASSIGN);
    PRODUCTIONS[i].rhs[2] = N(NT_EXPR);
    PRODUCTIONS[i].len = 3; i++;

    /* 6: BLOCK -> { STMT_LIST } */
    PRODUCTIONS[i].lhs = NT_BLOCK;
    PRODUCTIONS[i].rhs[0] = T(T_LBRACE);
    PRODUCTIONS[i].rhs[1] = N(NT_STMT_LIST);
    PRODUCTIONS[i].rhs[2] = T(T_RBRACE);
    PRODUCTIONS[i].len = 3; i++;

    /* 7: EXPR -> TERM EXPR_TAIL */
    PRODUCTIONS[i].lhs = NT_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_TERM);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    /* 8: EXPR_TAIL -> + TERM EXPR_TAIL */
    PRODUCTIONS[i].lhs = NT_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_PLUS);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM);
    PRODUCTIONS[i].rhs[2] = N(NT_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    /* 9: EXPR_TAIL -> - TERM EXPR_TAIL */
    PRODUCTIONS[i].lhs = NT_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_MINUS);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM);
    PRODUCTIONS[i].rhs[2] = N(NT_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    /* 10: EXPR_TAIL -> ε */
    PRODUCTIONS[i].lhs = NT_EXPR_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    /* 11: TERM -> FACTOR TERM_TAIL */
    PRODUCTIONS[i].lhs = NT_TERM;
    PRODUCTIONS[i].rhs[0] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    /* 12: TERM_TAIL -> * FACTOR TERM_TAIL */
    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_MULT);
    PRODUCTIONS[i].rhs[1] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[2] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    /* 13: TERM_TAIL -> / FACTOR TERM_TAIL */
    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_DIV);
    PRODUCTIONS[i].rhs[1] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[2] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    /* 14: TERM_TAIL -> ε */
    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    /* 15: FACTOR -> id */
    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_ID);
    PRODUCTIONS[i].len = 1; i++;

    /* 16: FACTOR -> int */
    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_INT);
    PRODUCTIONS[i].len = 1; i++;

    /* 17: FACTOR -> ( EXPR ) */
    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[2] = T(T_RPAREN);
    PRODUCTIONS[i].len = 3; i++;

    NUM_PRODUCTIONS = i;
}