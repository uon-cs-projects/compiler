#include "parse_table.h"
#include "first_follow.h"
#include <stdio.h>

int parse_table[NONTERMINAL_COUNT][TOKEN_TYPE_COUNT];

bool build_parse_table(void) {
    bool is_ll1 = true;

    /* Initialise all entries to -1 (no production) */
    for (int i = 0; i < NONTERMINAL_COUNT; i++)
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++)
            parse_table[i][j] = -1;

    for (int p = 0; p < NUM_PRODUCTIONS; p++) {
        Production *prod = &PRODUCTIONS[p];
        int A = prod->lhs;

        bool out[SET_SIZE];
        bool has_eps = first_star(prod->rhs, prod->len, out);

        /* For each terminal t in FIRST*(rhs) */
        for (int t = 0; t < TOKEN_TYPE_COUNT; t++) {
            if (!out[t]) continue;
            if (parse_table[A][t] != -1) {
                fprintf(stderr,
                    "LL(1) CONFLICT: T[%s][%d] already has production %d,"
                    " cannot add production %d\n",
                    NONTERMINAL_NAMES[A], t, parse_table[A][t], p);
                is_ll1 = false;
            } else {
                parse_table[A][t] = p;
            }
        }

        /* If ε ∈ FIRST*(rhs), use FOLLOW[A] */
        if (has_eps || prod->len == 0) {
            for (int t = 0; t < TOKEN_TYPE_COUNT; t++) {
                if (!follow_sets[A][t]) continue;
                if (parse_table[A][t] != -1) {
                    fprintf(stderr,
                        "LL(1) CONFLICT (FOLLOW): T[%s][%d]\n",
                        NONTERMINAL_NAMES[A], t);
                    is_ll1 = false;
                } else {
                    parse_table[A][t] = p;
                }
            }
        }
    }
    return is_ll1;
}

void print_parse_table(void) {
    printf("--- PARSE TABLE (Only showing populated cells) ---\n");
    for (int i = 0; i < NONTERMINAL_COUNT; i++) {
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++) {
            if (parse_table[i][j] != -1) {
                printf("M[%s, %d] = Rule %d\n", NONTERMINAL_NAMES[i], j, parse_table[i][j]);
            }
        }
    }
}