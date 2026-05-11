/*
 * icg.h — Intermediate Code Generator (Member 4)
 *
 * Defines the Quad struct (four-field three-address instruction),
 * the InstrList dynamic array, and the full public API for T4.1-T4.5.
 *
 * Pipeline position:
 *   parser_parse() → semantic_analyse() → icg_gen_stmt() → icg_print_list()
 */

#ifndef ICG_H
#define ICG_H

#include "src/parser/parse_tree.h"
#include "src/symbol_table/symbol_table.h"

/* ── Quad ─────────────────────────────────────────────────────────────────
 * A single three-address instruction.  Any field that is unused for a
 * particular opcode is left as the empty string "".
 *
 * Examples (from lecture materials):
 *   op="ADD"      result="t1"  arg1="b"   arg2="c"
 *   op="IF_FALSE" result=""    arg1="t2"  arg2="L1"
 *   op="LABEL"    result="L1"  arg1=""    arg2=""
 *   op="ASSIGN"   result="a"   arg1="1"   arg2=""
 * ─────────────────────────────────────────────────────────────────────── */
#define ICG_FIELD_LEN 64

typedef struct {
    char op    [ICG_FIELD_LEN];
    char result[ICG_FIELD_LEN];
    char arg1  [ICG_FIELD_LEN];
    char arg2  [ICG_FIELD_LEN];
} Quad;

/* ── InstrList ────────────────────────────────────────────────────────────
 * Dynamic array of Quads.  Grows automatically via realloc().
 * ─────────────────────────────────────────────────────────────────────── */
#define INSTR_LIST_INIT_CAP 256

typedef struct {
    Quad *quads;
    int   count;
    int   capacity;
} InstrList;

/* ── Instruction list API ─────────────────────────────────────────────── */

/* Allocate and initialise an empty InstrList. */
InstrList *icg_list_create(void);

/* Free all memory owned by the list. */
void icg_list_free(InstrList *list);

/*
 * Append one quad to the list.  Any of op/result/arg1/arg2 may be ""
 * to indicate an unused field.  The strings are copied into the Quad.
 */
void icg_emit(InstrList *list,
              const char *op,
              const char *result,
              const char *arg1,
              const char *arg2);

/* ── Name generators (T4.2) ───────────────────────────────────────────── */

/*
 * Write the next unique temporary name into buf (e.g. "t0", "t1", …).
 * Uses a static counter that is never reset so temporaries never collide
 * across function bodies in the same compilation unit.
 */
void icg_new_temp(char *buf, int buf_len);

/*
 * Write the next unique label name into buf (e.g. "L0", "L1", …).
 * Same single global counter as icg_new_temp's sibling counter.
 */
void icg_new_label(char *buf, int buf_len);

/* ── Tree walkers (T4.3 / T4.4) ──────────────────────────────────────── */

/*
 * icg_gen_expr()
 *
 * Walk an expression parse-tree node and emit the necessary quads.
 * On return, result_buf holds the name of the temporary or variable that
 * contains the expression's value (e.g. "t3", "x", "42").
 *
 * Handles: EXPR, AND_EXPR, NOT_EXPR, COMP_EXPR, MATH_EXPR, TERM, FACTOR
 * and their respective _TAIL nonterminals.
 */
void icg_gen_expr(const ParseTreeNode *node, InstrList *list,
                  char *result_buf, int buf_len);

/*
 * icg_gen_stmt()
 *
 * Walk a statement or program-level parse-tree node and emit quads.
 * Handles: PROGRAM, PROGRAM_TAIL, ELEMENT, FUNCTION, BLOCK, STMT_LIST,
 *          STMT (assign, if, while, return), IF_STMT, IF_TAIL.
 */
void icg_gen_stmt(const ParseTreeNode *node, InstrList *list);

/* ── Output (T4.5) ────────────────────────────────────────────────────── */

/*
 * Print all quads in the list to stdout as a formatted table:
 *
 *   === ICG QUADRUPLES ===
 *   ------------------------------------------------------------
 *     #   OP          RESULT      ARG1        ARG2
 *   ------------------------------------------------------------
 *     0   MUL         t0          b           2
 *   ...
 *   ------------------------------------------------------------
 *   N instruction(s) generated.
 */
void icg_print_list(const InstrList *list);

#endif /* ICG_H */
