#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"
#include <stdbool.h>

#define SET_SIZE  (TOKEN_TYPE_COUNT + 1)   /* +1 for ε slot */
#define EPS_SLOT  TOKEN_TYPE_COUNT 

extern bool first_sets [NONTERMINAL_COUNT][SET_SIZE];
extern bool follow_sets[NONTERMINAL_COUNT][SET_SIZE];

void compute_first (void);
void compute_follow(void);

/* FIRST*(sequence) — applies FIRST* rules to a symbol sequence.
 * Fills 'out' (size SET_SIZE). Returns true if ε is in the result. */
bool first_star(const Symbol *seq, int len, bool *out);

void print_first_sets (void);
void print_follow_sets(void);

#endif /* FIRST_FOLLOW_H */