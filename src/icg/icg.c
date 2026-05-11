/*
 * icg.c — Intermediate Code Generator (Member 4)
 * Implements T4.1 – T4.5.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icg.h"
#include "src/error_handler/error_handler.h"
#include "src/lexer/tokens.h"

/* ══════════════════════════════════════════════════════════════
 * T4.1  Instruction list management
 * ══════════════════════════════════════════════════════════════ */

InstrList *icg_list_create(void)
{
    InstrList *list = (InstrList *)malloc(sizeof(InstrList));
    if (!list) { report_error(0,"ICG","Out of memory (InstrList)"); exit(1); }
    list->quads    = (Quad *)malloc(INSTR_LIST_INIT_CAP * sizeof(Quad));
    if (!list->quads) { report_error(0,"ICG","Out of memory (quads)"); exit(1); }
    list->count    = 0;
    list->capacity = INSTR_LIST_INIT_CAP;
    return list;
}

void icg_list_free(InstrList *list)
{
    if (!list) return;
    free(list->quads);
    free(list);
}

void icg_emit(InstrList *list,
              const char *op,
              const char *result,
              const char *arg1,
              const char *arg2)
{
    if (!list) return;
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->quads = (Quad *)realloc(list->quads,
                                      list->capacity * sizeof(Quad));
        if (!list->quads) {
            report_error(0,"ICG","Out of memory (realloc quads)");
            exit(1);
        }
    }
    Quad *q = &list->quads[list->count++];
    snprintf(q->op,     ICG_FIELD_LEN, "%s", op     ? op     : "");
    snprintf(q->result, ICG_FIELD_LEN, "%s", result ? result : "");
    snprintf(q->arg1,   ICG_FIELD_LEN, "%s", arg1   ? arg1   : "");
    snprintf(q->arg2,   ICG_FIELD_LEN, "%s", arg2   ? arg2   : "");
}

/* ══════════════════════════════════════════════════════════════
 * T4.2  Temporary variable and label generators
 * ══════════════════════════════════════════════════════════════ */

static int temp_counter  = 0;
static int label_counter = 0;

void icg_new_temp(char *buf, int buf_len)
{
    snprintf(buf, buf_len, "t%d", temp_counter++);
}

void icg_new_label(char *buf, int buf_len)
{
    snprintf(buf, buf_len, "L%d", label_counter++);
}

/* ══════════════════════════════════════════════════════════════
 * Internal helpers
 * ══════════════════════════════════════════════════════════════ */

/* True if node represents an epsilon (empty) production */
static int is_empty_node(const ParseTreeNode *n)
{
    if (!n) return 1;
    if (!n->is_terminal && strcmp(n->symbol, "eps") == 0) return 1;
    if (!n->is_terminal && n->num_children == 0)          return 1;
    return 0;
}

/* True if a tail nonterminal has no real content */
static int is_empty_tail(const ParseTreeNode *n)
{
    if (!n) return 1;
    if (n->num_children == 0) return 1;
    if (n->num_children == 1 && is_empty_node(n->children[0])) return 1;
    return 0;
}

/* Find first non-terminal child with a given symbol name */
static const ParseTreeNode *find_nt(const ParseTreeNode *node, const char *sym)
{
    if (!node) return NULL;
    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *c = node->children[i];
        if (c && !c->is_terminal && strcmp(c->symbol, sym) == 0)
            return c;
    }
    return NULL;
}

/* ══════════════════════════════════════════════════════════════
 * T4.3  Expression code generation — forward declarations
 * ══════════════════════════════════════════════════════════════ */

/* Generate ARG_LIST params and return count */
static int icg_gen_arglist(const ParseTreeNode *node, InstrList *list);

/* Generate TERM_TAIL, carrying accumulated left result */
static void icg_gen_term_tail(const ParseTreeNode *tail, InstrList *list,
                               const char *left, char *result_buf, int buf_len);

/* Generate MATH_EXPR_TAIL, carrying accumulated left result */
static void icg_gen_math_tail(const ParseTreeNode *tail, InstrList *list,
                               const char *left, char *result_buf, int buf_len);

/* ── FACTOR ──────────────────────────────────────────────────── */
static void icg_gen_factor(const ParseTreeNode *node, InstrList *list,
                            char *result_buf, int buf_len)
{
    if (!node || is_empty_node(node)) { strncpy(result_buf, "", buf_len); return; }

    if (node->num_children == 0) { strncpy(result_buf, "", buf_len); return; }

    const ParseTreeNode *first = node->children[0];
    if (!first) { strncpy(result_buf, "", buf_len); return; }

    /* FACTOR → ( EXPR ) */
    if (first->is_terminal && first->token.id == T_LPAREN) {
        const ParseTreeNode *expr = find_nt(node, "EXPR");
        if (expr) icg_gen_expr(expr, list, result_buf, buf_len);
        else      strncpy(result_buf, "", buf_len);
        return;
    }

    /* FACTOR → int | bool | str literal */
    if (first->is_terminal &&
        (first->token.id == T_INT ||
         first->token.id == T_BOOL ||
         first->token.id == T_STR)) {
        strncpy(result_buf, first->token.lexeme, buf_len);
        return;
    }

    /* FACTOR → id FACTOR_ID_TAIL */
    if (first->is_terminal && first->token.id == T_ID) {
        const char *id_name = first->token.lexeme;

        const ParseTreeNode *tail =
            (node->num_children >= 2) ? node->children[1] : NULL;

        /* FACTOR_ID_TAIL → ε  →  plain variable reference */
        if (!tail || is_empty_tail(tail)) {
            strncpy(result_buf, id_name, buf_len);
            return;
        }

        const ParseTreeNode *tail_first =
            (tail->num_children > 0) ? tail->children[0] : NULL;

        /* FACTOR_ID_TAIL → ( ARG_LIST )  →  function call */
        if (tail_first && tail_first->is_terminal &&
            tail_first->token.id == T_LPAREN) {

            const ParseTreeNode *arglist = find_nt(tail, "ARG_LIST");
            int n_args = icg_gen_arglist(arglist, list);

            char t[ICG_FIELD_LEN];
            icg_new_temp(t, sizeof(t));
            char n_str[16];
            snprintf(n_str, sizeof(n_str), "%d", n_args);
            icg_emit(list, "CALL", t, id_name, n_str);
            strncpy(result_buf, t, buf_len);
            return;
        }

        /* Fallback: plain name */
        strncpy(result_buf, id_name, buf_len);
        return;
    }

    /* Fallback: recurse into first child */
    icg_gen_expr(first, list, result_buf, buf_len);
}

/* ── ARG_LIST param emitter ─────────────────────────────────── */
static int icg_gen_arglist(const ParseTreeNode *node, InstrList *list)
{
    if (!node || is_empty_tail(node)) return 0;

    /* ARG_LIST → EXPR ARG_LIST_TAIL */
    if (node->num_children == 0) return 0;

    int count = 0;
    const ParseTreeNode *expr_node = NULL;

    /* First child should be EXPR */
    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *c = node->children[i];
        if (!c || is_empty_node(c)) continue;
        if (!c->is_terminal && strcmp(c->symbol, "EXPR") == 0) {
            expr_node = c; break;
        }
    }
    if (expr_node) {
        char t[ICG_FIELD_LEN];
        icg_gen_expr(expr_node, list, t, sizeof(t));
        icg_emit(list, "PARAM", "", t, "");
        count++;
    }

    /* Walk ARG_LIST_TAIL */
    const ParseTreeNode *tail = find_nt(node, "ARG_LIST_TAIL");
    while (tail && !is_empty_tail(tail)) {
        const ParseTreeNode *e2 = find_nt(tail, "EXPR");
        if (e2) {
            char t2[ICG_FIELD_LEN];
            icg_gen_expr(e2, list, t2, sizeof(t2));
            icg_emit(list, "PARAM", "", t2, "");
            count++;
        }
        tail = find_nt(tail, "ARG_LIST_TAIL");
    }
    return count;
}

/* ── TERM_TAIL ────────────────────────────────────────────────── */
static void icg_gen_term_tail(const ParseTreeNode *tail, InstrList *list,
                               const char *left, char *result_buf, int buf_len)
{
    if (!tail || is_empty_tail(tail)) {
        strncpy(result_buf, left, buf_len);
        return;
    }

    /* TERM_TAIL → (* | /) FACTOR TERM_TAIL */
    const ParseTreeNode *op_node =
        (tail->num_children > 0) ? tail->children[0] : NULL;
    const ParseTreeNode *factor  =
        (tail->num_children > 1) ? tail->children[1] : NULL;
    const ParseTreeNode *next    =
        (tail->num_children > 2) ? tail->children[2] : NULL;

    if (!op_node || !op_node->is_terminal || !factor) {
        strncpy(result_buf, left, buf_len);
        return;
    }

    const char *op_str = (op_node->token.id == T_MULT) ? "MUL" : "DIV";

    char right[ICG_FIELD_LEN];
    icg_gen_expr(factor, list, right, sizeof(right));

    char t[ICG_FIELD_LEN];
    icg_new_temp(t, sizeof(t));
    icg_emit(list, op_str, t, left, right);

    icg_gen_term_tail(next, list, t, result_buf, buf_len);
}

/* ── TERM ────────────────────────────────────────────────────── */
static void icg_gen_term(const ParseTreeNode *node, InstrList *list,
                          char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    char left[ICG_FIELD_LEN];
    icg_gen_expr(node->children[0], list, left, sizeof(left));  /* FACTOR */

    const ParseTreeNode *tail =
        (node->num_children >= 2) ? node->children[1] : NULL;
    icg_gen_term_tail(tail, list, left, result_buf, buf_len);
}

/* ── MATH_EXPR_TAIL ──────────────────────────────────────────── */
static void icg_gen_math_tail(const ParseTreeNode *tail, InstrList *list,
                               const char *left, char *result_buf, int buf_len)
{
    if (!tail || is_empty_tail(tail)) {
        strncpy(result_buf, left, buf_len);
        return;
    }

    /* MATH_EXPR_TAIL → (+ | -) TERM MATH_EXPR_TAIL */
    const ParseTreeNode *op_node =
        (tail->num_children > 0) ? tail->children[0] : NULL;
    const ParseTreeNode *term    =
        (tail->num_children > 1) ? tail->children[1] : NULL;
    const ParseTreeNode *next    =
        (tail->num_children > 2) ? tail->children[2] : NULL;

    if (!op_node || !op_node->is_terminal || !term) {
        strncpy(result_buf, left, buf_len); return;
    }

    const char *op_str = (op_node->token.id == T_PLUS) ? "ADD" : "SUB";

    char right[ICG_FIELD_LEN];
    icg_gen_term(term, list, right, sizeof(right));

    char t[ICG_FIELD_LEN];
    icg_new_temp(t, sizeof(t));
    icg_emit(list, op_str, t, left, right);

    icg_gen_math_tail(next, list, t, result_buf, buf_len);
}

/* ── MATH_EXPR ───────────────────────────────────────────────── */
static void icg_gen_math_expr(const ParseTreeNode *node, InstrList *list,
                               char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    char left[ICG_FIELD_LEN];
    icg_gen_term(node->children[0], list, left, sizeof(left));

    const ParseTreeNode *tail =
        (node->num_children >= 2) ? node->children[1] : NULL;
    icg_gen_math_tail(tail, list, left, result_buf, buf_len);
}

/* ── COMP_EXPR ───────────────────────────────────────────────── */
static void icg_gen_comp_expr(const ParseTreeNode *node, InstrList *list,
                               char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    char left[ICG_FIELD_LEN];
    icg_gen_math_expr(node->children[0], list, left, sizeof(left));

    const ParseTreeNode *tail =
        (node->num_children >= 2) ? node->children[1] : NULL;

    if (!tail || is_empty_tail(tail)) {
        strncpy(result_buf, left, buf_len); return;
    }

    /* COMP_EXPR_TAIL → op MATH_EXPR */
    const ParseTreeNode *op_node =
        (tail->num_children > 0) ? tail->children[0] : NULL;
    const ParseTreeNode *rhs     =
        (tail->num_children > 1) ? tail->children[1] : NULL;

    if (!op_node || !op_node->is_terminal || !rhs) {
        strncpy(result_buf, left, buf_len); return;
    }

    const char *op_str;
    switch (op_node->token.id) {
        case T_EQ:  op_str = "EQ";  break;
        case T_NEQ: op_str = "NEQ"; break;
        case T_LT:  op_str = "LT";  break;
        case T_GT:  op_str = "GT";  break;
        case T_LE:  op_str = "LE";  break;
        case T_GE:  op_str = "GE";  break;
        default:    op_str = "CMP"; break;
    }

    char right[ICG_FIELD_LEN];
    icg_gen_math_expr(rhs, list, right, sizeof(right));

    char t[ICG_FIELD_LEN];
    icg_new_temp(t, sizeof(t));
    icg_emit(list, op_str, t, left, right);
    strncpy(result_buf, t, buf_len);
}

/* ── NOT_EXPR ────────────────────────────────────────────────── */
static void icg_gen_not_expr(const ParseTreeNode *node, InstrList *list,
                              char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    const ParseTreeNode *first = node->children[0];

    /* NOT_EXPR → not NOT_EXPR */
    if (first && first->is_terminal && first->token.id == T_NOT) {
        const ParseTreeNode *inner =
            (node->num_children >= 2) ? node->children[1] : NULL;
        char sub[ICG_FIELD_LEN];
        if (inner) icg_gen_not_expr(inner, list, sub, sizeof(sub));
        else        strncpy(sub, "", sizeof(sub));

        char t[ICG_FIELD_LEN];
        icg_new_temp(t, sizeof(t));
        icg_emit(list, "NOT", t, sub, "");
        strncpy(result_buf, t, buf_len);
        return;
    }

    /* NOT_EXPR → COMP_EXPR */
    icg_gen_comp_expr(first, list, result_buf, buf_len);
}

/* ── AND_EXPR ────────────────────────────────────────────────── */
static void icg_gen_and_expr(const ParseTreeNode *node, InstrList *list,
                              char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    char left[ICG_FIELD_LEN];
    icg_gen_not_expr(node->children[0], list, left, sizeof(left));

    const ParseTreeNode *tail =
        (node->num_children >= 2) ? node->children[1] : NULL;

    if (!tail || is_empty_tail(tail)) {
        strncpy(result_buf, left, buf_len); return;
    }

    /* AND_EXPR_TAIL → and NOT_EXPR AND_EXPR_TAIL
     * Short-circuit: t = left; IF_FALSE t GOTO L_done; t = right; LABEL L_done */
    char t[ICG_FIELD_LEN];
    icg_new_temp(t, sizeof(t));
    char L_done[ICG_FIELD_LEN];
    icg_new_label(L_done, sizeof(L_done));

    icg_emit(list, "ASSIGN", t, left, "");
    icg_emit(list, "IF_FALSE", "", t, L_done);

    const ParseTreeNode *right_not =
        (tail->num_children >= 2) ? tail->children[1] : NULL;
    char right[ICG_FIELD_LEN];
    if (right_not) icg_gen_not_expr(right_not, list, right, sizeof(right));
    else            strncpy(right, "", sizeof(right));

    icg_emit(list, "ASSIGN", t, right, "");

    /* Recurse into AND_EXPR_TAIL's own tail */
    const ParseTreeNode *next_tail =
        (tail->num_children >= 3) ? tail->children[2] : NULL;
    if (!is_empty_tail(next_tail)) {
        const ParseTreeNode *next_right =
            (next_tail && next_tail->num_children >= 2)
                ? next_tail->children[1] : NULL;
        if (next_right) {
            char r2[ICG_FIELD_LEN];
            icg_new_label(L_done, sizeof(L_done));
            icg_emit(list, "IF_FALSE", "", t, L_done);
            icg_gen_not_expr(next_right, list, r2, sizeof(r2));
            icg_emit(list, "ASSIGN", t, r2, "");
        }
    }

    icg_emit(list, "LABEL", L_done, "", "");
    strncpy(result_buf, t, buf_len);
}

/* ── EXPR (OR) ───────────────────────────────────────────────── */
static void icg_gen_or_expr(const ParseTreeNode *node, InstrList *list,
                             char *result_buf, int buf_len)
{
    if (!node || node->num_children == 0) {
        strncpy(result_buf, "", buf_len); return;
    }
    char left[ICG_FIELD_LEN];
    icg_gen_and_expr(node->children[0], list, left, sizeof(left));

    const ParseTreeNode *tail =
        (node->num_children >= 2) ? node->children[1] : NULL;

    if (!tail || is_empty_tail(tail)) {
        strncpy(result_buf, left, buf_len); return;
    }

    /* EXPR_TAIL → or AND_EXPR EXPR_TAIL
     * Short-circuit: t = left; IF t GOTO L_done; t = right; LABEL L_done */
    char t[ICG_FIELD_LEN];
    icg_new_temp(t, sizeof(t));
    char L_done[ICG_FIELD_LEN];
    icg_new_label(L_done, sizeof(L_done));

    icg_emit(list, "ASSIGN", t, left, "");
    icg_emit(list, "IF", "", t, L_done);

    const ParseTreeNode *right_and =
        (tail->num_children >= 2) ? tail->children[1] : NULL;
    char right[ICG_FIELD_LEN];
    if (right_and) icg_gen_and_expr(right_and, list, right, sizeof(right));
    else            strncpy(right, "", sizeof(right));

    icg_emit(list, "ASSIGN", t, right, "");
    icg_emit(list, "LABEL", L_done, "", "");
    strncpy(result_buf, t, buf_len);
}

/* ══════════════════════════════════════════════════════════════
 * icg_gen_expr — public dispatcher
 * ══════════════════════════════════════════════════════════════ */
void icg_gen_expr(const ParseTreeNode *node, InstrList *list,
                  char *result_buf, int buf_len)
{
    if (!node) { strncpy(result_buf, "", buf_len); return; }

    /* Terminal leaf — literal or identifier */
    if (node->is_terminal) {
        strncpy(result_buf, node->token.lexeme, buf_len);
        return;
    }

    const char *sym = node->symbol;

    if      (strcmp(sym, "FACTOR")    == 0) icg_gen_factor   (node, list, result_buf, buf_len);
    else if (strcmp(sym, "TERM")      == 0) icg_gen_term     (node, list, result_buf, buf_len);
    else if (strcmp(sym, "MATH_EXPR") == 0) icg_gen_math_expr(node, list, result_buf, buf_len);
    else if (strcmp(sym, "COMP_EXPR") == 0) icg_gen_comp_expr(node, list, result_buf, buf_len);
    else if (strcmp(sym, "NOT_EXPR")  == 0) icg_gen_not_expr (node, list, result_buf, buf_len);
    else if (strcmp(sym, "AND_EXPR")  == 0) icg_gen_and_expr (node, list, result_buf, buf_len);
    else if (strcmp(sym, "EXPR")      == 0) icg_gen_or_expr  (node, list, result_buf, buf_len);
    else if (node->num_children == 1)
        icg_gen_expr(node->children[0], list, result_buf, buf_len);
    else
        strncpy(result_buf, "", buf_len);
}

/* ======================================================
 * T4.4  Statement and control-flow code generation
 * ====================================================== */

/* Forward declaration � allows icg_gen_if_stmt to call icg_gen_stmt */
void icg_gen_stmt(const ParseTreeNode *node, InstrList *list);

/* Generate quads for IF_STMT and IF_TAIL nodes */
static void icg_gen_if_stmt(const ParseTreeNode *node, InstrList *list)
{
    if (!node) return;
    const ParseTreeNode *expr_node  = NULL;
    const ParseTreeNode *block_node = NULL;
    const ParseTreeNode *tail_node  = NULL;

    for (int i = 0; i < node->num_children; i++) {
        const ParseTreeNode *c = node->children[i];
        if (!c || is_empty_node(c) || c->is_terminal) continue;
        if      (strcmp(c->symbol,"EXPR")   ==0 && !expr_node)  expr_node  = c;
        else if (strcmp(c->symbol,"BLOCK")  ==0 && !block_node) block_node = c;
        else if (strcmp(c->symbol,"IF_TAIL")==0)                tail_node  = c;
    }

    char cond[ICG_FIELD_LEN] = "";
    if (expr_node) icg_gen_expr(expr_node, list, cond, sizeof(cond));

    int has_tail = tail_node && !is_empty_tail(tail_node);

    if (!has_tail) {
        /* if E { S }  =>  IF_FALSE cond L_after; S; LABEL L_after */
        char L_after[ICG_FIELD_LEN];
        icg_new_label(L_after, sizeof(L_after));
        icg_emit(list, "IF_FALSE", "", cond, L_after);
        if (block_node) icg_gen_stmt(block_node, list);
        icg_emit(list, "LABEL", L_after, "", "");
        return;
    }

    const ParseTreeNode *tf =
        (tail_node->num_children > 0) ? tail_node->children[0] : NULL;
    int is_elseif = tf && tf->is_terminal && tf->token.id == T_ELSEIF;

    char L_else[ICG_FIELD_LEN]; icg_new_label(L_else, sizeof(L_else));
    char L_end [ICG_FIELD_LEN]; icg_new_label(L_end,  sizeof(L_end));

    /* if E { S1 } else/elseif ... */
    icg_emit(list, "IF_FALSE", "", cond, L_else);
    if (block_node) icg_gen_stmt(block_node, list);
    icg_emit(list, "GOTO", "", "", L_end);
    icg_emit(list, "LABEL", L_else, "", "");

    if (is_elseif) {
        /* Treat IF_TAIL as a nested conditional */
        icg_gen_stmt(tail_node, list);
    } else {
        /* else { S2 } */
        const ParseTreeNode *else_blk = find_nt(tail_node, "BLOCK");
        if (else_blk) icg_gen_stmt(else_blk, list);
    }
    icg_emit(list, "LABEL", L_end, "", "");
}

/* Generate quads for while EXPR BLOCK */
static void icg_gen_while_stmt(const ParseTreeNode *node, InstrList *list)
{
    char L_begin[ICG_FIELD_LEN]; icg_new_label(L_begin, sizeof(L_begin));
    char L_after[ICG_FIELD_LEN]; icg_new_label(L_after, sizeof(L_after));

    icg_emit(list, "LABEL", L_begin, "", "");

    char cond[ICG_FIELD_LEN] = "";
    const ParseTreeNode *e = find_nt(node, "EXPR");
    if (e) icg_gen_expr(e, list, cond, sizeof(cond));

    icg_emit(list, "IF_FALSE", "", cond, L_after);
    const ParseTreeNode *b = find_nt(node, "BLOCK");
    if (b) icg_gen_stmt(b, list);
    icg_emit(list, "GOTO", "", "", L_begin);
    icg_emit(list, "LABEL", L_after, "", "");
}

/* Generate quads for for ID = EXPR BLOCK */
static void icg_gen_for_stmt(const ParseTreeNode *node, InstrList *list)
{
    char L_begin[ICG_FIELD_LEN]; icg_new_label(L_begin, sizeof(L_begin));
    char L_end[ICG_FIELD_LEN]; icg_new_label(L_end, sizeof(L_end));

    /* Find the loop variable and start expression */
    const char *loop_var = "i"; /* default */
    for (int i = 0; i < node->num_children; i++) {
        if (node->children[i] && node->children[i]->is_terminal &&
            node->children[i]->token.id == T_ID) {
            loop_var = node->children[i]->token.lexeme;
            break;
        }
    }

    /* Emit: var = start_expr */
    char start_val[ICG_FIELD_LEN] = "";
    const ParseTreeNode *e = find_nt(node, "EXPR");
    if (e) icg_gen_expr(e, list, start_val, sizeof(start_val));
    icg_emit(list, "ASSIGN", loop_var, start_val, "");

    /* LABEL L_begin */
    icg_emit(list, "LABEL", L_begin, "", "");

    /* IF_FALSE condition GOTO L_end (simplified: just continue for now) */
    icg_emit(list, "IF_FALSE", "", "1", L_end); /* dummy condition */

    /* Emit block code */
    const ParseTreeNode *b = find_nt(node, "BLOCK");
    if (b) icg_gen_stmt(b, list);

    /* Increment and loop back */
    char t_inc[ICG_FIELD_LEN]; icg_new_temp(t_inc, sizeof(t_inc));
    icg_emit(list, "ADD", t_inc, loop_var, "1");
    icg_emit(list, "ASSIGN", loop_var, t_inc, "");
    icg_emit(list, "GOTO", "", "", L_begin);
    icg_emit(list, "LABEL", L_end, "", "");
}

/* Main statement dispatcher */
void icg_gen_stmt(const ParseTreeNode *node, InstrList *list)
{
    if (!node || is_empty_node(node) || node->is_terminal) return;
    const char *sym = node->symbol;

    /* Program-level nodes: recurse all children */
    if (strcmp(sym,"PROGRAM")==0 || strcmp(sym,"PROGRAM_TAIL")==0 ||
        strcmp(sym,"ELEMENT")==0) {
        for (int i = 0; i < node->num_children; i++)
            icg_gen_stmt(node->children[i], list);
        return;
    }

    /* FUNCTION => FUNC_BEGIN / BLOCK / FUNC_END */
    if (strcmp(sym,"FUNCTION")==0) {
        const char *fn = "?";
        for (int i = 0; i < node->num_children; i++) {
            const ParseTreeNode *c = node->children[i];
            if (c && c->is_terminal && c->token.id == T_ID) {
                fn = c->token.lexeme; break;
            }
        }
        icg_emit(list, "FUNC_BEGIN", fn, "", "");
        const ParseTreeNode *blk = find_nt(node, "BLOCK");
        if (blk) icg_gen_stmt(blk, list);
        icg_emit(list, "FUNC_END", fn, "", "");
        return;
    }

    /* BLOCK => recurse STMT_LIST */
    if (strcmp(sym,"BLOCK")==0) {
        const ParseTreeNode *sl = find_nt(node, "STMT_LIST");
        if (sl) icg_gen_stmt(sl, list);
        return;
    }

    /* STMT_LIST => recurse all children */
    if (strcmp(sym,"STMT_LIST")==0) {
        for (int i = 0; i < node->num_children; i++)
            icg_gen_stmt(node->children[i], list);
        return;
    }

    /* IF_STMT / IF_TAIL */
    if (strcmp(sym,"IF_STMT")==0 || strcmp(sym,"IF_TAIL")==0) {
        icg_gen_if_stmt(node, list); return;
    }
    /* WHILE_STMT => while EXPR BLOCK */
    if (strcmp(sym,"WHILE_STMT")==0) {
        icg_gen_while_stmt(node, list); return;
    }

    /* FOR_STMT => for ID = EXPR BLOCK */
    if (strcmp(sym,"FOR_STMT")==0) {
        icg_gen_for_stmt(node, list); return;
    }

    /* PRINT_STMT => print EXPR */
    if (strcmp(sym,"PRINT_STMT")==0) {
        char val[ICG_FIELD_LEN] = "";
        const ParseTreeNode *e = find_nt(node, "EXPR");
        if (e) icg_gen_expr(e, list, val, sizeof(val));
        icg_emit(list, "PRINT", "", val, "");
        return;
    }

    /* BREAK_STMT => break */
    if (strcmp(sym,"BREAK_STMT")==0) {
        icg_emit(list, "BREAK", "", "", "");
        return;
    }
    /* STMT */
    if (strcmp(sym,"STMT")==0) {
        if (node->num_children == 0) return;
        const ParseTreeNode *first = node->children[0];
        if (!first) return;

        /* STMT => IF_STMT (non-terminal first child) */
        if (!first->is_terminal && strcmp(first->symbol,"IF_STMT")==0) {
            icg_gen_if_stmt(first, list); return;
        }
        if (!first->is_terminal) { icg_gen_stmt(first, list); return; }

        /* STMT => return EXPR */
        if (first->token.id == T_RETURN) {
            const ParseTreeNode *re =
                (node->num_children >= 2) ? node->children[1] : NULL;
            char ret[ICG_FIELD_LEN] = "";
            if (re) icg_gen_expr(re, list, ret, sizeof(ret));
            icg_emit(list, "RETURN", "", ret, "");
            return;
        }

        /* STMT => id STMT_ID_TAIL */
        if (first->token.id == T_ID) {
            const char *id_name = first->token.lexeme;
            const ParseTreeNode *tail =
                (node->num_children >= 2) ? node->children[1] : NULL;
            if (!tail || is_empty_tail(tail)) return;
            const ParseTreeNode *top =
                (tail->num_children > 0) ? tail->children[0] : NULL;
            if (!top) return;

            /* Assignment: id = EXPR */
            if (top->is_terminal && top->token.id == T_ASSIGN) {
                const ParseTreeNode *rhs =
                    (tail->num_children >= 2) ? tail->children[1] : NULL;
                char val[ICG_FIELD_LEN] = "";
                if (rhs) icg_gen_expr(rhs, list, val, sizeof(val));
                icg_emit(list, "ASSIGN", id_name, val, "");
                return;
            }
            /* Function-call statement: id ( args ) */
            if (top->is_terminal && top->token.id == T_LPAREN) {
                const ParseTreeNode *al = find_nt(tail, "ARG_LIST");
                int n = icg_gen_arglist(al, list);
                char ns[16]; snprintf(ns, sizeof(ns), "%d", n);
                icg_emit(list, "CALL", "", id_name, ns);
                return;
            }
        }
        return;
    }

    /* Default: recurse all children */
    for (int i = 0; i < node->num_children; i++)
        icg_gen_stmt(node->children[i], list);
}

/* ======================================================
 * T4.5  ICG printer
 * ====================================================== */

void icg_print_list(const InstrList *list)
{
    if (!list) return;
    printf("------------------------------------------------------------\n");
    printf("  %-4s  %-10s  %-10s  %-10s  %-10s\n",
           "#","OP","RESULT","ARG1","ARG2");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < list->count; i++) {
        const Quad *q = &list->quads[i];
        printf("  %-4d  %-10s  %-10s  %-10s  %-10s\n", i,
               q->op[0]     ? q->op     : "",
               q->result[0] ? q->result : "",
               q->arg1[0]   ? q->arg1   : "",
               q->arg2[0]   ? q->arg2   : "");
    }
    printf("------------------------------------------------------------\n");
    printf("  %d instruction(s) generated.\n", list->count);
}
