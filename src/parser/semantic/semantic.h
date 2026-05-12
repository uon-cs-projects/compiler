#ifndef SEMANTIC_H
#define SEMANTIC_H

/*
 * semantic.h — Semantic Analysis & Type Checker
 * Member 3: Declaration Processor, Expression & Statement Type-Checker
 *
 * Depends on:
 *   - Member 1: error_handler (report_error / report_warning)
 *   - Member 2: type_system   (TypeNode, type_equivalent, type_make_*)
 *   - Symbol table: symbol_table.h (scope_enter, scope_exit, symbol_insert,
 *                                   symbol_lookup)
 *   - Parser:       parse_tree.h   (ParseTreeNode)
 */

#include "src/parser/parse_tree.h"
#include "src/symbol_table/symbol_table.h"
#include "src/symbol_table/type_system.h"

/* ── Semantic error codes (extend error_context.h's numeric space) ──────── */
/* Place in the 2000-range to avoid collision with parser error codes.       */
#define ERR_UNDECLARED    2001   /* Use of identifier before declaration      */
#define ERR_REDECLARED    2002   /* Identifier declared twice in same scope   */
#define ERR_TYPE_MISMATCH 2003   /* Incompatible types in expression/assign   */
#define ERR_NOT_ARRAY     2004   /* Subscript applied to non-array type       */
#define ERR_NOT_POINTER   2005   /* Dereference applied to non-pointer type   */

/* ── Entry point ─────────────────────────────────────────────────────────── */

/*
 * semantic_analyse()
 *
 * Walk the full parse tree produced by the parser, populate the symbol table,
 * and type-check every expression and statement.
 *
 * Parameters:
 *   root  – root ParseTreeNode* returned by parser_parse()
 *   table – initialised SymbolTable* (caller owns; call symbol_table_init first)
 *
 * Returns:
 *   0  on success (no semantic errors detected)
 *  >0  number of semantic errors found (compilation should be aborted)
 *
 * Side-effects:
 *   - Calls scope_enter() / scope_exit() around every BLOCK node.
 *   - Calls symbol_insert() for every declaration found.
 *   - Calls report_error() through Member 1's error_handler for every error.
 */
int semantic_analyse(ParseTreeNode *root, SymbolTable *table);

/* ── Declaration helpers ─────────────────────────────────────────────────── */

/*
 * type_from_parse_node()
 *
 * Build a TypeNode from the subtree rooted at a type-descriptor node.
 * The grammar represents types through token kinds in the FACTOR / EXPR
 * hierarchy; this function maps those tokens to TypeNode structures.
 *
 * The caller does NOT own the returned TypeNode — it is owned by the symbol
 * record that consumes it via symbol_insert().  If you need to free it on an
 * error path before insertion, call type_free() yourself.
 *
 * Returns NULL and reports an error when the subtree is not a recognised type.
 */
TypeNode *type_from_parse_node(const ParseTreeNode *type_node);

/* ── Expression type-checker ─────────────────────────────────────────────── */

/*
 * typecheck_expr()
 *
 * Recursively infer and verify the type of any expression node.
 *
 * Returns a FRESHLY ALLOCATED TypeNode* describing the expression's type,
 * or a TK_ERROR TypeNode when a type error is detected (so that checking
 * can continue and accumulate multiple errors without crashing).
 *
 * The caller is responsible for calling type_free() on the returned TypeNode
 * once it is no longer needed.
 *
 * Implemented rules (T3.2):
 *   FACTOR → T_INT   ⟹ TK_INT
 *   FACTOR → T_STR   ⟹ TK_STRING
 *   FACTOR → T_BOOL  ⟹ TK_BOOL
 *   FACTOR → T_ID    ⟹ symbol_lookup → declared type, else ERR_UNDECLARED
 *   MATH_EXPR / TERM with +,-,*,/ ⟹ both sides must be INT or FLOAT;
 *                                    mixed INT/FLOAT ⟹ ERR_TYPE_MISMATCH
 *   FACTOR → T_ID [ EXPR ] ⟹ E1 must be TK_ARRAY, E2 must be TK_INT
 *                              → base_type of E1; else ERR_NOT_ARRAY
 *   COMP_EXPR / logical ops  ⟹ TK_BOOL
 */
TypeNode *typecheck_expr(const ParseTreeNode *node, SymbolTable *table);

/* ── Statement type-checker ──────────────────────────────────────────────── */

/*
 * typecheck_stmt()
 *
 * Type-check a single statement node.  Statements do not produce a value;
 * on success the function returns a TK_VOID TypeNode.  On error it still
 * returns TK_VOID so that checking can continue.
 *
 * Implemented rules (T3.3):
 *   STMT → id STMT_ID_TAIL (assignment):
 *     look up id's declared type; typecheck_expr(RHS);
 *     verify type_equivalent(declared, expr_type); else ERR_TYPE_MISMATCH.
 *   STMT → IF_STMT:
 *     typecheck_expr(condition) must be TK_BOOL or TK_INT.
 *   STMT → while EXPR BLOCK:
 *     typecheck_expr(condition) must be TK_BOOL or TK_INT.
 *   STMT → return EXPR:
 *     typecheck_expr(EXPR); result is recorded but not checked here
 *     (function-return matching belongs to a future pass).
 *
 * The returned TypeNode* is always TK_VOID and must be freed by the caller.
 */
TypeNode *typecheck_stmt(const ParseTreeNode *node, SymbolTable *table);

#endif /* SEMANTIC_H */