#include "parse_tree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Static node pool ──────────────────────────────────────────── */
static ParseTreeNode pool[NODE_POOL_SIZE];
static int           pool_top = 0;

void tree_reset_pool(void) { pool_top = 0; }

static ParseTreeNode *alloc_node(void) {
    if (pool_top >= NODE_POOL_SIZE) {
        fprintf(stderr, "ParseTree: node pool exhausted\n");
        exit(1);
    }
    ParseTreeNode *n = &pool[pool_top++];
    memset(n, 0, sizeof(ParseTreeNode));
    return n;
}

/* ── Constructors ──────────────────────────────────────────────── */

ParseTreeNode *node_new_terminal(const Token *tok) {
    ParseTreeNode *n = alloc_node();
    n->is_terminal = true;
    n->token       = *tok;
    /* Display as  LEXEME  for leaf nodes */
    snprintf(n->symbol, sizeof(n->symbol), "%s", tok->lexeme);
    return n;
}

ParseTreeNode *node_new_internal(const char *symbol_name) {
    ParseTreeNode *n = alloc_node();
    n->is_terminal = false;
    snprintf(n->symbol, sizeof(n->symbol), "%s", symbol_name);
    return n;
}

void node_add_child(ParseTreeNode *parent, ParseTreeNode *child) {
    if (parent->num_children < MAX_CHILDREN)
        parent->children[parent->num_children++] = child;
}

/* ── Pretty-printer ────────────────────────────────────────────── */
/*
 * Prints like:
 *   └── PROGRAM
 *       └── STMT_LIST
 *           ├── STMT
 *           │   └── [id] "x"
 *           └── STMT_LIST
 *               └── ε
 */
void print_tree(const ParseTreeNode *node,
                const char *prefix, bool is_last) {
    if (!node) return;

    printf("%s%s", prefix, is_last ? "└── " : "├── ");

    if (node->is_terminal)
        /* Show token type index alongside the lexeme */
        printf("[tok:%d] \"%s\"\n", node->token.id, node->token.lexeme);
    else
        printf("%s\n", node->symbol);

    /* Build the prefix for children */
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix),
             "%s%s", prefix, is_last ? "    " : "│   ");

    for (int i = 0; i < node->num_children; i++)
        print_tree(node->children[i], new_prefix,
                   i == node->num_children - 1);
}