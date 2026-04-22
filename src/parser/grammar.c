#include "grammar.h"

const char *NONTERMINAL_NAMES[NONTERMINAL_COUNT] = {
    "PROGRAM", "PROGRAM_TAIL", "ELEMENT", "FUNCTION",
    "PARAM_LIST", "PARAM_LIST_TAIL", "BLOCK",
    "STMT_LIST", "STMT", "STMT_ID_TAIL",
    "IF_STMT", "IF_TAIL",
    "EXPR", "EXPR_TAIL",
    "AND_EXPR", "AND_EXPR_TAIL",
    "NOT_EXPR",
    "COMP_EXPR", "COMP_EXPR_TAIL",
    "MATH_EXPR", "MATH_EXPR_TAIL",
    "TERM", "TERM_TAIL",
    "FACTOR", "FACTOR_ID_TAIL",
    "ARG_LIST", "ARG_LIST_TAIL"
};

Production PRODUCTIONS[MAX_PRODUCTIONS];
int        NUM_PRODUCTIONS = 0;

#define T(tok)  ((Symbol){ SYM_TERMINAL,    (tok)  })
#define N(nt)   ((Symbol){ SYM_NONTERMINAL, (nt)   })
#define EPS     ((Symbol){ SYM_EPSILON,     0      })

void grammar_init(void) {
    int i = 0;

    PRODUCTIONS[i].lhs = NT_PROGRAM;
    PRODUCTIONS[i].rhs[0] = N(NT_ELEMENT);
    PRODUCTIONS[i].rhs[1] = N(NT_PROGRAM_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_PROGRAM_TAIL;
    PRODUCTIONS[i].rhs[0] = N(NT_ELEMENT);
    PRODUCTIONS[i].rhs[1] = N(NT_PROGRAM_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_PROGRAM_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_ELEMENT;
    PRODUCTIONS[i].rhs[0] = N(NT_FUNCTION);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_ELEMENT;
    PRODUCTIONS[i].rhs[0] = N(NT_STMT);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_FUNCTION;
    PRODUCTIONS[i].rhs[0] = T(T_DEF);
    PRODUCTIONS[i].rhs[1] = T(T_ID);
    PRODUCTIONS[i].rhs[2] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[3] = N(NT_PARAM_LIST);
    PRODUCTIONS[i].rhs[4] = T(T_RPAREN);
    PRODUCTIONS[i].rhs[5] = N(NT_BLOCK);
    PRODUCTIONS[i].len = 6; i++;

    PRODUCTIONS[i].lhs = NT_PARAM_LIST;
    PRODUCTIONS[i].rhs[0] = T(T_ID);
    PRODUCTIONS[i].rhs[1] = N(NT_PARAM_LIST_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_PARAM_LIST;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_PARAM_LIST_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_COMMA);
    PRODUCTIONS[i].rhs[1] = T(T_ID);
    PRODUCTIONS[i].rhs[2] = N(NT_PARAM_LIST_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_PARAM_LIST_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_BLOCK;
    PRODUCTIONS[i].rhs[0] = T(T_LBRACE);
    PRODUCTIONS[i].rhs[1] = N(NT_STMT_LIST);
    PRODUCTIONS[i].rhs[2] = T(T_RBRACE);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_STMT_LIST;
    PRODUCTIONS[i].rhs[0] = N(NT_STMT);
    PRODUCTIONS[i].rhs[1] = N(NT_STMT_LIST);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_STMT_LIST;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = T(T_ID);
    PRODUCTIONS[i].rhs[1] = N(NT_STMT_ID_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = N(NT_IF_STMT);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_STMT;
    PRODUCTIONS[i].rhs[0] = T(T_RETURN);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_STMT_ID_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_ASSIGN);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_STMT_ID_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[1] = N(NT_ARG_LIST);
    PRODUCTIONS[i].rhs[2] = T(T_RPAREN);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_IF_STMT;
    PRODUCTIONS[i].rhs[0] = T(T_IF);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[2] = N(NT_BLOCK);
    PRODUCTIONS[i].rhs[3] = N(NT_IF_TAIL);
    PRODUCTIONS[i].len = 4; i++;

    PRODUCTIONS[i].lhs = NT_IF_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_ELSE);
    PRODUCTIONS[i].rhs[1] = N(NT_BLOCK);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_IF_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_ELSEIF);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[2] = N(NT_BLOCK);
    PRODUCTIONS[i].rhs[3] = N(NT_IF_TAIL);
    PRODUCTIONS[i].len = 4; i++;

    PRODUCTIONS[i].lhs = NT_IF_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_AND_EXPR);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_OR);
    PRODUCTIONS[i].rhs[1] = N(NT_AND_EXPR);
    PRODUCTIONS[i].rhs[2] = N(NT_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_EXPR_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_AND_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_NOT_EXPR);
    PRODUCTIONS[i].rhs[1] = N(NT_AND_EXPR_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_AND_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_AND);
    PRODUCTIONS[i].rhs[1] = N(NT_NOT_EXPR);
    PRODUCTIONS[i].rhs[2] = N(NT_AND_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_AND_EXPR_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_NOT_EXPR;
    PRODUCTIONS[i].rhs[0] = T(T_NOT);
    PRODUCTIONS[i].rhs[1] = N(NT_NOT_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_NOT_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_COMP_EXPR);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].rhs[1] = N(NT_COMP_EXPR_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_EQ);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_NEQ);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_GT);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_LT);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_GE);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_LE);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR);
    PRODUCTIONS[i].len = 2; i++;
    
    PRODUCTIONS[i].lhs = NT_COMP_EXPR_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_MATH_EXPR;
    PRODUCTIONS[i].rhs[0] = N(NT_TERM);
    PRODUCTIONS[i].rhs[1] = N(NT_MATH_EXPR_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_MATH_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_PLUS);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM);
    PRODUCTIONS[i].rhs[2] = N(NT_MATH_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_MATH_EXPR_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_MINUS);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM);
    PRODUCTIONS[i].rhs[2] = N(NT_MATH_EXPR_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_MATH_EXPR_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_TERM;
    PRODUCTIONS[i].rhs[0] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[1] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_MULT);
    PRODUCTIONS[i].rhs[1] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[2] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_DIV);
    PRODUCTIONS[i].rhs[1] = N(NT_FACTOR);
    PRODUCTIONS[i].rhs[2] = N(NT_TERM_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_TERM_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_ID);
    PRODUCTIONS[i].rhs[1] = N(NT_FACTOR_ID_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_INT);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_STR);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_BOOL);
    PRODUCTIONS[i].len = 1; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR;
    PRODUCTIONS[i].rhs[0] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[2] = T(T_RPAREN);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR_ID_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_LPAREN);
    PRODUCTIONS[i].rhs[1] = N(NT_ARG_LIST);
    PRODUCTIONS[i].rhs[2] = T(T_RPAREN);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_FACTOR_ID_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_ARG_LIST;
    PRODUCTIONS[i].rhs[0] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[1] = N(NT_ARG_LIST_TAIL);
    PRODUCTIONS[i].len = 2; i++;

    PRODUCTIONS[i].lhs = NT_ARG_LIST;
    PRODUCTIONS[i].len = 0; i++;

    PRODUCTIONS[i].lhs = NT_ARG_LIST_TAIL;
    PRODUCTIONS[i].rhs[0] = T(T_COMMA);
    PRODUCTIONS[i].rhs[1] = N(NT_EXPR);
    PRODUCTIONS[i].rhs[2] = N(NT_ARG_LIST_TAIL);
    PRODUCTIONS[i].len = 3; i++;

    PRODUCTIONS[i].lhs = NT_ARG_LIST_TAIL;
    PRODUCTIONS[i].len = 0; i++;

    NUM_PRODUCTIONS = i;
}