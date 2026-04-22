#include "parser.h"
#include "parse_table.h"
#include "../error_handler/error_handler.h"
#include <stdio.h>
#include <string.h>

/* ── Stack helpers ─────────────────────────────────────────────── */
static ParserStack stk;

static void stack_push(GrammarSymbol sym, ParseTreeNode *node) {
    if (stk.top >= STACK_SIZE - 1) {
        fprintf(stderr, "Parser: stack overflow\n");
        return;
    }
    stk.sym_stack [++stk.top] = sym;
    stk.tree_stack[  stk.top] = node;
}

static GrammarSymbol         stack_peek_sym (void) { return stk.sym_stack [stk.top]; }
static ParseTreeNode *stack_peek_node(void) { return stk.tree_stack[stk.top]; }
static void           stack_pop      (void) { if (stk.top >= 0) stk.top--;    }

/* ── Error helpers (wrappers around error_handler) ─────────────── */

static void match_error(int expected_tok, const Token *got) {
    char msg[256];
    snprintf(msg, sizeof(msg),
             "Expected token type %d but got '%s' (type %d)",
             expected_tok, got->lexeme, got->id);
    report_error(got->line, "SYNTAX", msg);
}

static void predict_error(int nt_idx, const Token *got) {
    char msg[256];
    snprintf(msg, sizeof(msg),
             "No production for '%s' with lookahead '%s' (type %d)",
             NONTERMINAL_NAMES[nt_idx], got->lexeme, got->id);
    report_error(got->line, "SYNTAX", msg);
}

/* ── Main parse function ───────────────────────────────────────── */
ParseTreeNode *parser_parse(TokenStream *ts) {
    /* Initialise stack:  push $  then  push start-symbol  */
    stk.top = -1;
    GrammarSymbol eof_sym   = { SYM_TERMINAL,    TOK_EOF    };
    GrammarSymbol start_sym = { SYM_NONTERMINAL, NT_PROGRAM };

    ParseTreeNode *root = node_new_internal(NONTERMINAL_NAMES[NT_PROGRAM]);

    stack_push(eof_sym,   NULL); /* bottom sentinel */
    stack_push(start_sym, root);

    Token *la = ts_peek(ts);    /* lookahead */

    while (stk.top >= 0) {
        GrammarSymbol         top      = stack_peek_sym();
        ParseTreeNode *top_node = stack_peek_node();

        /* ── SUCCESS: both stack and input are at EOF ── */
        if (top.kind  == SYM_TERMINAL &&
            top.index == TOK_EOF      &&
            la->id  == TOK_EOF) {
            return root;
        }

        /* ── MATCH: top is a terminal ─────────────────── */
        if (top.kind == SYM_TERMINAL) {
            if (top.index == (int)la->id) {
                /* Fill in the leaf node with the actual token */
                if (top_node) {
                    top_node->is_terminal = true;
                    top_node->token       = *la;
                    snprintf(top_node->symbol, sizeof(top_node->symbol),
                             "%s", la->lexeme);
                }
                stack_pop();
                ts_consume(ts);
                la = ts_peek(ts);
            } else {
                match_error(top.index, la);
                stack_pop(); /* pop the terminal and try to continue */
                continue;
            }
            continue;
        }

        /* ── PREDICT: top is a nonterminal ───────────── */
        if (top.kind == SYM_NONTERMINAL) {
            int prod_idx = parse_table[top.index][la->id];

            if (prod_idx == -1) {
                predict_error(top.index, la);
                // Use your recovery function!
                error_recover(top.index, ts); 
                
                // Update lookahead after recovery
                la = ts_peek(ts);
                
                // Pop the non-terminal that failed so we can try the next thing on stack
                stack_pop(); 
                continue;
            }

            stack_pop(); /* pop the nonterminal */

            Production *prod = &PRODUCTIONS[prod_idx];

            /* Create child nodes and push RHS right-to-left */
            /* (right-to-left push means left-to-right processing) */
            ParseTreeNode *children[MAX_RHS_LENGTH];
            for (int i = 0; i < prod->len; i++) {
                GrammarSymbol s = prod->rhs[i];
                if (s.kind == SYM_EPSILON)
                    children[i] = node_new_internal("eps");
                else if (s.kind == SYM_TERMINAL)
                    children[i] = node_new_internal("?"); /* filled on MATCH */
                else
                    children[i] = node_new_internal(NONTERMINAL_NAMES[s.index]);

                node_add_child(top_node, children[i]);
            }

            /* Push right-to-left so leftmost symbol is processed first */
            for (int i = prod->len - 1; i >= 0; i--)
                stack_push(prod->rhs[i], children[i]);

            /* ε-production: nothing to push, add leaf and continue */
            if (prod->len == 0) {
                ParseTreeNode *eps = node_new_internal("eps");
                node_add_child(top_node, eps);
            }
            continue;
        }

        /* ── EPSILON symbol on stack: just pop ────────── */
        if (top.kind == SYM_EPSILON) {
            stack_pop();
            continue;
        }
    }

    /* Stack drained but input not at EOF */
    if (la->id != TOK_EOF) {
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "Unexpected token '%s' after end of program", la->lexeme);
        report_error(la->line, "SYNTAX", msg);
    }

    return root;
}
