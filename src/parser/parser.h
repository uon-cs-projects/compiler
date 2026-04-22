#ifndef PARSER_H
#define PARSER_H

#include "parse_tree.h"
#include "token_stream.h"
#include "grammar.h"

#define STACK_SIZE 512

typedef struct {
    GrammarSymbol         sym_stack [STACK_SIZE]; /* grammar symbols         */
    ParseTreeNode *tree_stack[STACK_SIZE]; /* parallel tree nodes     */
    int            top;                    /* index of top (-1=empty) */
} ParserStack;

/*
 * Run the LL(1) parse.
 * Returns: root of parse tree on success, NULL on any error.
 */
ParseTreeNode *parser_parse(TokenStream *ts);

#endif /* PARSER_H */