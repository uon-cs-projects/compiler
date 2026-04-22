#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "grammar.h"
#include <stdbool.h>

#define MAX_CHILDREN  10
#define NODE_POOL_SIZE 2048

typedef struct ParseTreeNode {
    char                  symbol[64];     /* grammar symbol name        */
    bool                  is_terminal;
    Token                 token;          /* valid only when terminal   */
    struct ParseTreeNode *children[MAX_CHILDREN];
    int                   num_children;
} ParseTreeNode;

/* Node constructors (use a static pool — no malloc in the hot path) */
ParseTreeNode *node_new_terminal(const Token *tok);
ParseTreeNode *node_new_internal(const char  *symbol_name);
void           node_add_child   (ParseTreeNode *parent,
                                  ParseTreeNode *child);

/* Recursive pretty-printer; call with prefix="" and is_last=true */
void print_tree(const ParseTreeNode *node,
                const char *prefix, bool is_last);

/* Reset the node pool (call between parses if reusing) */
void tree_reset_pool(void);

#endif /* PARSE_TREE_H */