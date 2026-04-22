#include "first_follow.h"
#include <string.h>
#include <stdio.h>

bool first_sets [NONTERMINAL_COUNT][SET_SIZE];
bool follow_sets[NONTERMINAL_COUNT][SET_SIZE];

/* Implement FIRST*(alpha) */
bool first_star(const Symbol *seq, int len, bool *out) {
    memset(out, 0, SET_SIZE * sizeof(bool));
    
    if (len == 0) {
        out[EPS_SLOT] = true;
        return true;
    }
    
    bool all_derive_eps = true;
    for (int i = 0; i < len; i++) {
        if (seq[i].kind == SYM_TERMINAL) {
            out[seq[i].index] = true;
            all_derive_eps = false;
            break;
        } else if (seq[i].kind == SYM_EPSILON) {
            continue;
        } else {
            int nt = seq[i].index;
            /* Copy all non-ε from FIRST[nt] */
            for (int t = 0; t < TOKEN_TYPE_COUNT; t++) {
                if (first_sets[nt][t]) out[t] = true;
            }
            if (!first_sets[nt][EPS_SLOT]) {
                all_derive_eps = false;
                break;
            }
        }
    }
    
    if (all_derive_eps) {
        out[EPS_SLOT] = true;
    }
    return all_derive_eps;
}

void compute_first(void) {
    memset(first_sets, 0, sizeof(first_sets));

    bool changed = true;
    while (changed) {
        changed = false;
        for (int p = 0; p < NUM_PRODUCTIONS; p++) {
            Production *prod = &PRODUCTIONS[p];
            int nt = prod->lhs;

            if (prod->len == 0) {
                if (!first_sets[nt][EPS_SLOT]) {
                    first_sets[nt][EPS_SLOT] = true;
                    changed = true;
                }
                continue;
            }

            bool rhs_out[SET_SIZE];
            bool has_eps = first_star(prod->rhs, prod->len, rhs_out);

            for (int t = 0; t < TOKEN_TYPE_COUNT; t++) {
                if (rhs_out[t] && !first_sets[nt][t]) {
                    first_sets[nt][t] = true;
                    changed = true;
                }
            }
            if (has_eps && !first_sets[nt][EPS_SLOT]) {
                first_sets[nt][EPS_SLOT] = true;
                changed = true;
            }
        }
    }
}

void compute_follow(void) {
    memset(follow_sets, 0, sizeof(follow_sets));

    /* Start symbol gets $ */
    follow_sets[NT_PROGRAM][TOK_EOF] = true;

    bool changed = true;
    while (changed) {
        changed = false;
        for (int p = 0; p < NUM_PRODUCTIONS; p++) {
            Production *prod = &PRODUCTIONS[p];
            int B = prod->lhs;

            for (int i = 0; i < prod->len; i++) {
                if (prod->rhs[i].kind != SYM_NONTERMINAL) continue;
                int A = prod->rhs[i].index;

                /* ω = everything after position i */
                const Symbol *omega = &prod->rhs[i + 1];
                int omega_len = prod->len - i - 1;

                bool omega_first[SET_SIZE];
                bool has_eps = first_star(omega, omega_len, omega_first);

                for (int t = 0; t < TOKEN_TYPE_COUNT; t++) {
                    if (omega_first[t] && !follow_sets[A][t]) {
                        follow_sets[A][t] = true;
                        changed = true;
                    }
                }
                
                /* If ω can derive ε, FOLLOW[B] ⊆ FOLLOW[A] */
                if (has_eps || omega_len == 0) {
                    for (int t = 0; t <= TOKEN_TYPE_COUNT; t++) {
                        if (follow_sets[B][t] && !follow_sets[A][t]) {
                            follow_sets[A][t] = true;
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

void print_first_sets(void) {
    printf("--- FIRST SETS ---\n");
    for(int i = 0; i < NONTERMINAL_COUNT; i++) {
        printf("%10s : { ", NONTERMINAL_NAMES[i]);
        for(int t=0; t<TOKEN_TYPE_COUNT; t++) {
            if(first_sets[i][t]) printf("%d ", t);
        }
        if(first_sets[i][EPS_SLOT]) printf("EPS ");
        printf("}\n");
    }
}

void print_follow_sets(void) {
    printf("--- FOLLOW SETS ---\n");
    for(int i = 0; i < NONTERMINAL_COUNT; i++) {
        printf("%10s : { ", NONTERMINAL_NAMES[i]);
        for(int t=0; t<TOKEN_TYPE_COUNT; t++) {
            if(follow_sets[i][t]) printf("%d ", t);
        }
        if(follow_sets[i][TOK_EOF]) printf("EOF ");
        printf("}\n");
    }
}