#ifndef PARSE_TABLE_H
#define PARSE_TABLE_H

#include "grammar.h"
#include <stdbool.h>

/*
 * parse_table[nonterminal_index][terminal_token_type] = production index
 * Value -1 means no entry (error).
 */
extern int parse_table[NONTERMINAL_COUNT][TOKEN_TYPE_COUNT];

/* Returns true if grammar is LL(1) (no conflicts), false otherwise. */
bool build_parse_table(void);

void print_parse_table(void);

#endif /* PARSE_TABLE_H */