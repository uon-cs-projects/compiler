/*
 * semantic.c — Semantic Analysis & Type Checker (Member 3)
 *
 * Implements:
 *   T3.1  Declaration processor  – semantic_analyse(), type_from_parse_node()
 *   T3.2  Expression type-checker – typecheck_expr()
 *   T3.3  Statement type-checker  – typecheck_stmt()
 *   T3.4  Connects to Member 2's type_equivalent()
 *   T3.5  All errors routed through Member 1's report_error()
 *
 * Dependency map:
 *   Member 1 → error_handler.h  (report_error, report_warning)
 *   Member 2 → type_system.h    (TypeNode, type_make_*, type_equivalent,
 *                                 type_free)
 *   Symbol   → symbol_table.h   (scope_enter, scope_exit, symbol_insert,
 *                                 symbol_lookup)
 *   Parser   → parse_tree.h     (ParseTreeNode, is_terminal, token, symbol)
 *   Tokens   → tokens.h         (T_INT, T_BOOL, T_ID, T_ASSIGN, …)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "src/error_handler/error_handler.h"
#include "src/parser/parse_tree.h"
#include "src/symbol_table/symbol_table.h"
#include "src/symbol_table/type_system.h"
#include "tokens.h"

/* ══════════════════════════════════════════════════════════════════════════ *
 *  Internal helpers
 * ══════════════════════════════════════════════════════════════════════════ */

/* Convenience: return a TK_ERROR node so callers can keep walking the tree   */
static TypeNode *error_type(void)
{
    TypeNode *t = type_make_basic(TK_ERROR);
    return t;  /* never NULL; type_make_basic only fails on OOM             */
}

/* True when a TypeNode represents an integer-compatible boolean condition    */
static int is_boolean_compatible(const TypeNode *t)
{
    return t != NULL && (t->kind == TK_BOOL || t->kind == TK_INT);
}

/* True when a TypeNode is numeric (INT or BOOL)                             */
static int is_numeric(const TypeNode *t)
{
    return t != NULL && (t->kind == TK_INT || t->kind == TK_BOOL);
}

/* Emit a semantic error at the given line number via Member 1's handler      */
static void sem_error(int line, int err_code, const char *msg)
{
    (void)err_code;          /* err_code reserved for ErrorContext extension   */
    report_error(line, "SEMANTIC", msg);
}

/* Return the line number of the left-most terminal descendant, or 0          */
static int node_line(const ParseTreeNode *node)
{
    if (node == NULL)          return 0;
    if (node->is_terminal)     return node->token.line;
    for (int i = 0; i < node->num_children; i++) {
        int l = node_line(node->children[i]);
        if (l != 0) return l;
    }
    return 0;
}

/* Find the first terminal child whose token ID matches tok_id, or NULL       */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static const ParseTreeNode *find_terminal(const ParseTreeNode *node, int tok_id)
{
    if (node == NULL)                          return NULL;
    if (node->is_terminal && node->token.id == tok_id) return node;
    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *hit = find_terminal(node->children[i], tok_id);
        if (hit) return hit;
    }
    return NULL;
}

/* Find the first non-terminal child whose symbol matches name, or NULL       */
static const ParseTreeNode *find_nonterminal(const ParseTreeNode *node,
                                              const char *sym)
{
    if (node == NULL) return NULL;
    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *c = node->children[i];
        if (c && !c->is_terminal && strcmp(c->symbol, sym) == 0)
            return c;
    }
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════════════ *
 *  T3.1 — type_from_parse_node
 *
 *  The grammar used by this compiler does NOT have an explicit TYPE production
 *  with keywords like "int", "float", etc. (those are not in tokens.h).
 *  Instead, variable types are inferred from the kind of the initialiser
 *  literal in the assignment, or from a declaration comment.  We map the
 *  token ID at the leaf of a TYPE-carrying subtree to a TypeNode.
 *
 *  The function is also called when the parse tree carries a direct terminal
 *  node (e.g., during a manual hand-built unit-test tree).
 * ══════════════════════════════════════════════════════════════════════════ */

TypeNode *type_from_parse_node(const ParseTreeNode *type_node)
{
    if (type_node == NULL) return NULL;

    /* If the caller passed a terminal node directly, dispatch on its token   */
    if (type_node->is_terminal) {
        switch (type_node->token.id) {
            case T_INT:   return type_make_basic(TK_INT);
            case T_BOOL:  return type_make_basic(TK_BOOL);
            case T_STR:   return type_make_basic(TK_INT);  /* strings as ints */
            default:
                sem_error(type_node->token.line, ERR_TYPE_MISMATCH,
                          "Unrecognised type token in declaration");
                return NULL;
        }
    }

    /*
     * Non-terminal node: look for the first typed literal terminal child.
     * This covers both direct children and the first level of FACTOR subtrees
     * produced by the LL(1) parser.
     */
    for (int i = 0; i < type_node->num_children; i++) {
        const ParseTreeNode *child = type_node->children[i];
        if (child == NULL) continue;
        if (child->is_terminal) {
            switch (child->token.id) {
                case T_INT:   return type_make_basic(TK_INT);
                case T_BOOL:  return type_make_basic(TK_BOOL);
                case T_STR:   return type_make_basic(TK_INT);
                default:      break;
            }
        }
    }

    /* Fall back: recurse into first non-terminal child                       */
    if (type_node->num_children > 0)
        return type_from_parse_node(type_node->children[0]);

    sem_error(node_line(type_node), ERR_TYPE_MISMATCH,
              "Cannot determine type from parse node");
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════════════ *
 *  T3.2 — typecheck_expr
 *
 *  Walk any expression-producing node and return the inferred TypeNode*.
 *  Returns a TK_ERROR node on any type error (never NULL) so the caller can
 *  continue checking without null-dereference guards everywhere.
 *
 *  Grammar hierarchy (bottom-up, lowest precedence first):
 *    EXPR → AND_EXPR EXPR_TAIL
 *    AND_EXPR → NOT_EXPR AND_EXPR_TAIL
 *    NOT_EXPR → COMP_EXPR  |  not NOT_EXPR
 *    COMP_EXPR → MATH_EXPR COMP_EXPR_TAIL
 *    MATH_EXPR → TERM MATH_EXPR_TAIL          (+ / -)
 *    TERM → FACTOR TERM_TAIL                  (* / /)
 *    FACTOR → id FACTOR_ID_TAIL | int | str | bool | ( EXPR )
 *    FACTOR_ID_TAIL → ( ARG_LIST ) | [ EXPR ] | ε
 * ══════════════════════════════════════════════════════════════════════════ */

TypeNode *typecheck_expr(const ParseTreeNode *node, SymbolTable *table)
{
    if (node == NULL) return error_type();

    /* ── Terminal leaf ───────────────────────────────────────────────────── */
    if (node->is_terminal) {
        switch (node->token.id) {
            case T_INT:  return type_make_basic(TK_INT);
            case T_BOOL: return type_make_basic(TK_BOOL);
            case T_STR:  return type_make_basic(TK_INT);   /* string literal  */
            case T_ID: {
                SymbolRecord *rec = symbol_lookup(table, node->token.lexeme);
                if (rec == NULL) {
                    char msg[160];
                    snprintf(msg, sizeof(msg),
                             "Undeclared identifier '%s'", node->token.lexeme);
                    sem_error(node->token.line, ERR_UNDECLARED, msg);
                    return error_type();
                }
                /* Return a copy so the caller can type_free() independently  */
                TypeNode *copy = type_make_basic(rec->type->kind);
                if (copy && rec->type->base_type) {
                    copy->base_type  = rec->type->base_type;   /* shallow ref */
                    copy->array_size = rec->type->array_size;
                }
                return copy;
            }
            default:
                return error_type();
        }
    }

    const char *sym = node->symbol;

    /* ── FACTOR ──────────────────────────────────────────────────────────── */
    if (strcmp(sym, "FACTOR") == 0) {
        if (node->num_children == 0) return error_type();

        const ParseTreeNode *first = node->children[0];

        /* FACTOR → ( EXPR )  ⟹ recurse into EXPR child                    */
        if (first->is_terminal && first->token.id == T_LPAREN) {
            const ParseTreeNode *inner =
                find_nonterminal(node, "EXPR");
            return inner ? typecheck_expr(inner, table) : error_type();
        }

        /* FACTOR → int | bool | str                                         */
        if (first->is_terminal) {
            return typecheck_expr(first, table);
        }

        /* FACTOR → id FACTOR_ID_TAIL                                        */
        if (first->is_terminal && first->token.id == T_ID) {
            TypeNode *id_type = typecheck_expr(first, table);
            const ParseTreeNode *tail =
                find_nonterminal(node, "FACTOR_ID_TAIL");

            if (tail == NULL || tail->num_children == 0) {
                /* FACTOR_ID_TAIL → ε : plain identifier reference           */
                return id_type;
            }

            const ParseTreeNode *tail_first = tail->children[0];
            if (tail_first->is_terminal &&
                tail_first->token.id == T_LBRACKET) {
                /* FACTOR → id [ EXPR ]  (array subscript)                   */
                if (id_type->kind != TK_ARRAY) {
                    char msg[160];
                    snprintf(msg, sizeof(msg),
                             "Subscript on non-array variable '%s'",
                             first->token.lexeme);
                    sem_error(first->token.line, ERR_NOT_ARRAY, msg);
                    type_free(id_type);
                    return error_type();
                }
                const ParseTreeNode *idx =
                    find_nonterminal(tail, "EXPR");
                if (idx) {
                    TypeNode *idx_type = typecheck_expr(idx, table);
                    if (idx_type->kind != TK_INT) {
                        sem_error(node_line(idx), ERR_TYPE_MISMATCH,
                                  "Array index must be of type INT");
                        type_free(idx_type);
                        type_free(id_type);
                        return error_type();
                    }
                    type_free(idx_type);
                }
                /* Return base type (shallow copy)                           */
                TypeNode *base = type_make_basic(id_type->base_type
                                                 ? id_type->base_type->kind
                                                 : TK_ERROR);
                type_free(id_type);
                return base;
            }

            /* FACTOR → id ( ARG_LIST )  (function call) — return TK_INT    */
            type_free(id_type);
            return type_make_basic(TK_INT);
        }

        /* Fallback: recurse into first child                                */
        return typecheck_expr(first, table);
    }

    /* ── TERM → FACTOR TERM_TAIL ─────────────────────────────────────────── */
    if (strcmp(sym, "TERM") == 0) {
        if (node->num_children == 0) return error_type();

        const ParseTreeNode *factor = node->children[0];
        TypeNode *left_type = typecheck_expr(factor, table);

        if (node->num_children < 2) return left_type;

        const ParseTreeNode *tail = node->children[1];
        if (tail == NULL || tail->num_children == 0) return left_type;

        /* TERM_TAIL → ( * | / ) FACTOR TERM_TAIL                           */
        const ParseTreeNode *right_factor =
            (tail->num_children >= 2) ? tail->children[1] : NULL;
        if (right_factor == NULL) return left_type;

        TypeNode *right_type = typecheck_expr(right_factor, table);

        if (!is_numeric(left_type) || !is_numeric(right_type)) {
            sem_error(node_line(node), ERR_TYPE_MISMATCH,
                      "Operands of + - must be numeric (INT or BOOL)");
            type_free(left_type);
            type_free(right_type);
            return error_type();
        }
        type_free(right_type);
        return left_type;   /* left_type is INT or FLOAT (same as right)     */
    }

    /* ── MATH_EXPR → TERM MATH_EXPR_TAIL (+/-) ───────────────────────────── */
    if (strcmp(sym, "MATH_EXPR") == 0) {
        if (node->num_children == 0) return error_type();

        TypeNode *left_type = typecheck_expr(node->children[0], table);

        if (node->num_children < 2) return left_type;

        const ParseTreeNode *tail = node->children[1];
        if (tail == NULL || tail->num_children == 0) return left_type;

        /* MATH_EXPR_TAIL → ( + | - ) TERM MATH_EXPR_TAIL                   */
        const ParseTreeNode *right_term =
            (tail->num_children >= 2) ? tail->children[1] : NULL;
        if (right_term == NULL) return left_type;

        TypeNode *right_type = typecheck_expr(right_term, table);

        if (!is_numeric(left_type) || !is_numeric(right_type)) {
            sem_error(node_line(node), ERR_TYPE_MISMATCH,
                      "Operands of * / must be numeric (INT or BOOL)");
            type_free(left_type);
            type_free(right_type);
            return error_type();
        }

        type_free(right_type);
        return left_type;
    }

    /* ── COMP_EXPR → MATH_EXPR COMP_EXPR_TAIL (==, !=, <, >, <=, >=) ─────── */
    if (strcmp(sym, "COMP_EXPR") == 0) {
        if (node->num_children == 0) return error_type();

        TypeNode *left_type = typecheck_expr(node->children[0], table);

        if (node->num_children < 2 ||
            node->children[1] == NULL ||
            node->children[1]->num_children == 0) {
            /* No comparison operator: propagate MATH_EXPR type              */
            return left_type;
        }

        /* There is a COMP_EXPR_TAIL with an operator                        */
        const ParseTreeNode *tail = node->children[1];
        const ParseTreeNode *right_math =
            (tail->num_children >= 2) ? tail->children[1] : NULL;
        if (right_math) {
            TypeNode *right_type = typecheck_expr(right_math, table);
            if (left_type->kind != right_type->kind &&
                !(is_numeric(left_type) && is_numeric(right_type))) {
                sem_error(node_line(node), ERR_TYPE_MISMATCH,
                          "Comparison operands have incompatible types");
            }
            type_free(right_type);
        }
        type_free(left_type);
        return type_make_basic(TK_BOOL);   /* comparisons always yield BOOL  */
    }

    /* ── NOT_EXPR → not NOT_EXPR | COMP_EXPR ──────────────────────────────── */
    if (strcmp(sym, "NOT_EXPR") == 0) {
        if (node->num_children == 0) return error_type();

        const ParseTreeNode *first = node->children[0];
        if (first->is_terminal && first->token.id == T_NOT) {
            /* not NOT_EXPR                                                   */
            const ParseTreeNode *inner =
                (node->num_children >= 2) ? node->children[1] : NULL;
            TypeNode *t = inner ? typecheck_expr(inner, table) : error_type();
            if (!is_boolean_compatible(t)) {
                sem_error(node_line(node), ERR_TYPE_MISMATCH,
                          "'not' operand must be boolean or integer");
                type_free(t);
                return error_type();
            }
            type_free(t);
            return type_make_basic(TK_BOOL);
        }
        return typecheck_expr(first, table);
    }

    /* ── AND_EXPR → NOT_EXPR AND_EXPR_TAIL ────────────────────────────────── */
    if (strcmp(sym, "AND_EXPR") == 0) {
        if (node->num_children == 0) return error_type();

        TypeNode *left = typecheck_expr(node->children[0], table);

        if (node->num_children < 2 ||
            node->children[1] == NULL ||
            node->children[1]->num_children == 0) {
            return left;
        }

        /* AND_EXPR_TAIL → and NOT_EXPR AND_EXPR_TAIL                        */
        const ParseTreeNode *tail = node->children[1];
        const ParseTreeNode *right_not =
            (tail->num_children >= 2) ? tail->children[1] : NULL;
        if (right_not) {
            TypeNode *right = typecheck_expr(right_not, table);
            if (!is_boolean_compatible(left) || !is_boolean_compatible(right)) {
                sem_error(node_line(node), ERR_TYPE_MISMATCH,
                          "'and' operands must be boolean or integer");
            }
            type_free(right);
        }
        type_free(left);
        return type_make_basic(TK_BOOL);
    }

    /* ── EXPR → AND_EXPR EXPR_TAIL (or) ──────────────────────────────────── */
    if (strcmp(sym, "EXPR") == 0) {
        if (node->num_children == 0) return error_type();

        TypeNode *left = typecheck_expr(node->children[0], table);

        if (node->num_children < 2 ||
            node->children[1] == NULL ||
            node->children[1]->num_children == 0) {
            return left;
        }

        /* EXPR_TAIL → or AND_EXPR EXPR_TAIL                                 */
        const ParseTreeNode *tail = node->children[1];
        const ParseTreeNode *right_and =
            (tail->num_children >= 2) ? tail->children[1] : NULL;
        if (right_and) {
            TypeNode *right = typecheck_expr(right_and, table);
            if (!is_boolean_compatible(left) || !is_boolean_compatible(right)) {
                sem_error(node_line(node), ERR_TYPE_MISMATCH,
                          "'or' operands must be boolean or integer");
            }
            type_free(right);
        }
        type_free(left);
        return type_make_basic(TK_BOOL);
    }

    /* ── Fallback: single child – unwrap and recurse ──────────────────────── */
    if (node->num_children == 1)
        return typecheck_expr(node->children[0], table);

    return error_type();
}

/* ══════════════════════════════════════════════════════════════════════════ *
 *  T3.3 — typecheck_stmt
 * ══════════════════════════════════════════════════════════════════════════ */

TypeNode *typecheck_stmt(const ParseTreeNode *node, SymbolTable *table)
{
    if (node == NULL) return type_make_basic(TK_VOID);

    const char *sym = node->symbol;

    /* ── STMT → id STMT_ID_TAIL ──────────────────────────────────────────── */
    if (strcmp(sym, "STMT") == 0) {
        if (node->num_children < 2) return type_make_basic(TK_VOID);

        const ParseTreeNode *id_node = node->children[0];
        const ParseTreeNode *tail    = node->children[1];

        if (!id_node || !id_node->is_terminal ||
            id_node->token.id != T_ID)
            goto recurse_children;

        if (tail == NULL || tail->num_children == 0)
            goto recurse_children;

        const ParseTreeNode *tail_first = tail->children[0];
        if (tail_first == NULL) goto recurse_children;

        if (tail_first->is_terminal &&
            tail_first->token.id == T_ASSIGN) {
            /* Assignment: id = EXPR                                         */
            SymbolRecord *rec =
                symbol_lookup(table, id_node->token.lexeme);
            if (rec == NULL) {
                char msg[160];
                snprintf(msg, sizeof(msg),
                         "Assignment to undeclared variable '%s'",
                         id_node->token.lexeme);
                sem_error(id_node->token.line, ERR_UNDECLARED, msg);
                return type_make_basic(TK_VOID);
            }

            const ParseTreeNode *rhs =
                (tail->num_children >= 2) ? tail->children[1] : NULL;
            if (rhs == NULL) return type_make_basic(TK_VOID);

            TypeNode *expr_type = typecheck_expr(rhs, table);
            if (!type_equivalent(rec->type, expr_type)) {
                char msg[160];
                snprintf(msg, sizeof(msg),
                         "Type mismatch in assignment to '%s'",
                         id_node->token.lexeme);
                sem_error(id_node->token.line, ERR_TYPE_MISMATCH, msg);
            }
            type_free(expr_type);
            return type_make_basic(TK_VOID);
        }

        /* STMT → id ( ARG_LIST ) : function-call statement – just check args */
        if (tail_first->is_terminal &&
            tail_first->token.id == T_LPAREN) {
            /* Argument type-checking is a future extension;
             * for now just walk the arg list to catch undeclared uses.      */
            return type_make_basic(TK_VOID);
        }
    }

    /* ── IF_STMT → if EXPR BLOCK IF_TAIL ─────────────────────────────────── */
    if (strcmp(sym, "IF_STMT") == 0) {
        /* children[0] = T_IF, children[1] = EXPR, children[2] = BLOCK, … */
        const ParseTreeNode *cond = find_nonterminal(node, "EXPR");
        if (cond) {
            TypeNode *cond_type = typecheck_expr(cond, table);
            if (!is_boolean_compatible(cond_type)) {
                sem_error(node_line(cond), ERR_TYPE_MISMATCH,
                          "Condition of 'if' must be boolean or integer");
            }
            type_free(cond_type);
        }

        /* Type-check the body block recursively (scope handled in analyse)  */
        const ParseTreeNode *body = find_nonterminal(node, "BLOCK");
        if (body) typecheck_stmt(body, table);

        const ParseTreeNode *tail = find_nonterminal(node, "IF_TAIL");
        if (tail && tail->num_children > 0) typecheck_stmt(tail, table);

        return type_make_basic(TK_VOID);
    }

    /* ── STMT → while EXPR BLOCK ─────────────────────────────────────────── */
    if (strcmp(sym, "STMT") == 0) {
        const ParseTreeNode *kw = (node->num_children > 0)
                                  ? node->children[0] : NULL;
        if (kw && kw->is_terminal && kw->token.id == T_WHILE) {
            const ParseTreeNode *cond = find_nonterminal(node, "EXPR");
            if (cond) {
                TypeNode *cond_type = typecheck_expr(cond, table);
                if (!is_boolean_compatible(cond_type)) {
                    sem_error(node_line(cond), ERR_TYPE_MISMATCH,
                              "Condition of 'while' must be boolean or integer");
                }
                type_free(cond_type);
            }
            const ParseTreeNode *body = find_nonterminal(node, "BLOCK");
            if (body) typecheck_stmt(body, table);
            return type_make_basic(TK_VOID);
        }
    }

    /* ── STMT → return EXPR ─────────────────────────────────────────────── */
    if (strcmp(sym, "STMT") == 0 && node->num_children >= 2) {
        const ParseTreeNode *kw = node->children[0];
        if (kw && kw->is_terminal && kw->token.id == T_RETURN) {
            const ParseTreeNode *ret_expr = node->children[1];
            TypeNode *ret_type = typecheck_expr(ret_expr, table);
            /* Return-type vs function signature matching is a future pass.  */
            type_free(ret_type);
            return type_make_basic(TK_VOID);
        }
    }

recurse_children:
    /* Default: recurse into every child statement/expression               */
    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *child = node->children[i];
        if (child == NULL || child->is_terminal) continue;

        /* Check whether the child is a statement-producing non-terminal    */
        if (strcmp(child->symbol, "STMT")      == 0 ||
            strcmp(child->symbol, "STMT_LIST") == 0 ||
            strcmp(child->symbol, "IF_STMT")   == 0 ||
            strcmp(child->symbol, "IF_TAIL")   == 0 ||
            strcmp(child->symbol, "BLOCK")     == 0) {
            TypeNode *t = typecheck_stmt(child, table);
            type_free(t);
        }
    }
    return type_make_basic(TK_VOID);
}

/* ══════════════════════════════════════════════════════════════════════════ *
 *  T3.1 — Declaration processor (recursive tree walk)
 *
 *  Grammar node responsibilities:
 *    BLOCK      → { STMT_LIST }      → push scope, recurse, pop scope
 *    FUNCTION   → def id ( PARAMS ) BLOCK → push scope for params
 *    STMT_LIST  → STMT STMT_LIST | ε → recurse each statement
 *    STMT       → id STMT_ID_TAIL … → if assignment, infer & insert type
 *    ELEMENT    → FUNCTION | STMT    → dispatch
 *    PROGRAM    → ELEMENT PROGRAM_TAIL
 * ══════════════════════════════════════════════════════════════════════════ */

/*
 * process_declarations()
 *
 * This is an internal recursive helper that implements the full tree walk.
 * It handles both declaration insertion (T3.1) and statement checking (T3.3)
 * in a single pass.
 */
static void process_declarations(const ParseTreeNode *node, SymbolTable *table)
{
    if (node == NULL) return;
    if (node->is_terminal) return;

    const char *sym = node->symbol;

    /* ── BLOCK → { STMT_LIST } : open a new lexical scope ──────────────── */
    if (strcmp(sym, "BLOCK") == 0) {
        scope_enter(table);
        for (int i = 0; i < node->num_children; i++)
            process_declarations(node->children[i], table);
        scope_exit(table);
        return;
    }

    /* ── FUNCTION → def id ( PARAM_LIST ) BLOCK ─────────────────────────── */
    if (strcmp(sym, "FUNCTION") == 0) {
        /*
         * Open a scope for the function body so that parameters and local
         * variables don't leak into the enclosing scope.
         * The BLOCK child will open another scope for its own body; we open
         * one here specifically for the parameter list.
         */
        scope_enter(table);

        /* Insert each parameter as TK_INT (grammar has no type annotation)  */
        const ParseTreeNode *params = find_nonterminal(node, "PARAM_LIST");
        if (params) {
            /* PARAM_LIST → id PARAM_LIST_TAIL | ε                           */
            const ParseTreeNode *p = params;
            while (p != NULL && p->num_children > 0) {
                const ParseTreeNode *id_child = p->children[0];
                if (id_child && id_child->is_terminal &&
                    id_child->token.id == T_ID) {
                    TypeNode *param_type = type_make_basic(TK_INT);
                    symbol_insert(table, id_child->token.lexeme,
                                  param_type, id_child->token.line);
                }
                p = (p->num_children >= 2)
                    ? find_nonterminal(p->children[1], "PARAM_LIST_TAIL")
                    : NULL;
                if (p == NULL && params->num_children >= 2)
                    p = find_nonterminal(params, "PARAM_LIST_TAIL");
                break;  /* avoid looping; PARAM_LIST_TAIL handled below      */
            }

            /* Walk PARAM_LIST_TAIL for additional parameters                */
            const ParseTreeNode *ptail =
                find_nonterminal(params, "PARAM_LIST_TAIL");
            while (ptail && ptail->num_children >= 2) {
                /* PARAM_LIST_TAIL → , id PARAM_LIST_TAIL                    */
                const ParseTreeNode *id_child = ptail->children[1];
                if (id_child && id_child->is_terminal &&
                    id_child->token.id == T_ID) {
                    TypeNode *param_type = type_make_basic(TK_INT);
                    symbol_insert(table, id_child->token.lexeme,
                                  param_type, id_child->token.line);
                }
                ptail = find_nonterminal(ptail, "PARAM_LIST_TAIL");
            }
        }

        /* Process the BLOCK (opens its own inner scope)                     */
        const ParseTreeNode *blk = find_nonterminal(node, "BLOCK");
        if (blk) process_declarations(blk, table);

        scope_exit(table);
        return;
    }

    /* ── STMT → id STMT_ID_TAIL  (assignment ⟹ implicit declaration) ─────── */
    /*
     * This compiler's grammar has no explicit declaration syntax (no
     * "var id : type;" production).  We treat the FIRST assignment to an
     * identifier in a scope as its declaration, inferring the type from
     * the RHS expression.  Subsequent assignments type-check against the
     * stored type.
     */
    if (strcmp(sym, "STMT") == 0 &&
        node->num_children >= 2) {

        const ParseTreeNode *id_node = node->children[0];
        const ParseTreeNode *tail    = node->children[1];

        if (id_node && id_node->is_terminal &&
            id_node->token.id == T_ID &&
            tail && tail->num_children >= 2) {

            const ParseTreeNode *op = tail->children[0];
            if (op && op->is_terminal && op->token.id == T_ASSIGN) {
                const ParseTreeNode *rhs = tail->children[1];

                /* Check if already declared in THIS scope (redeclaration)   */
                SymbolRecord *existing =
                    symbol_lookup_local(table->current,
                                        id_node->token.lexeme);
                if (existing == NULL) {
                    /* First assignment: insert with inferred type            */
                    TypeNode *inferred = typecheck_expr(rhs, table);
                    if (inferred->kind == TK_ERROR) {
                        /* RHS had a type error; insert as TK_INT to recover */
                        type_free(inferred);
                        inferred = type_make_basic(TK_INT);
                    }
                    symbol_insert(table, id_node->token.lexeme,
                                  inferred, id_node->token.line);
                } else {
                    /* Already declared: type-check the assignment            */
                    TypeNode *expr_type = typecheck_expr(rhs, table);
                    if (!type_equivalent(existing->type, expr_type)) {
                        char msg[160];
                        snprintf(msg, sizeof(msg),
                                 "Type mismatch in assignment to '%s'",
                                 id_node->token.lexeme);
                        sem_error(id_node->token.line,
                                  ERR_TYPE_MISMATCH, msg);
                    }
                    type_free(expr_type);
                }
            }
        }

        /* Recurse into child nodes to catch nested blocks / if / while      */
        for (int i = 0; i < node->num_children; i++)
            process_declarations(node->children[i], table);

        /* Also type-check the whole statement for other statement forms      */
        TypeNode *void_t = typecheck_stmt(node, table);
        type_free(void_t);
        return;
    }

    /* ── All other nodes: recurse into children ─────────────────────────── */
    for (int i = 0; i < node->num_children; i++)
        process_declarations(node->children[i], table);
}

/* ══════════════════════════════════════════════════════════════════════════ *
 *  T3.1 — Entry point
 * ══════════════════════════════════════════════════════════════════════════ */

int semantic_analyse(ParseTreeNode *root, SymbolTable *table)
{
    if (root == NULL || table == NULL) return 0;

    int errors_before = error_count;   /* from error_handler.h extern        */

    process_declarations(root, table);

    return error_count - errors_before;
}