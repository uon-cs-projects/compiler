/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/parser/shared_types.h"
#include "src/parser/grammar.h"
#include "src/parser/first_follow.h"
#include "src/parser/parse_table.h"
#include "src/parser/token_stream.h"
#include "src/parser/parser.h"
#include "src/parser/parse_tree.h"
#include "src/symbol_table/symbol_table.h"
#include "src/symbol_table/type_system.h"
#include "src/error_handler/error_handler.h"
#include "src/parser/semantic/semantic.h"
#include "src/icg/icg.h"
#include "lexer_adapter.h"

#define MAX_SOURCE 65536
#ifndef MAX_TOKENS
#define MAX_TOKENS 10000
#endif

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <source_file> [--tokens-only] [--debug]\n", prog);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    int tokens_only = 0;
    for (int i = 2; i < argc; i++)
        if (strcmp(argv[i], "--tokens-only") == 0) tokens_only = 1;

    /* ── 1. Read source file ─────────────────────────────────── */
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }
    char source[MAX_SOURCE];
    int  n = fread(source, 1, MAX_SOURCE - 1, f);
    source[n] = '\0';
    fclose(f);

    /* ── 2. Lex & adapt ─────────────────────────────────────── */
    Token tokens[MAX_TOKENS];
    int   tok_count = lexer_adapter_run(source, tokens, MAX_TOKENS);

    printf("=== LEXER OUTPUT ===\n");
    for (int i = 0; i < tok_count; i++) {
        /* tokens[i].lexeme holds standard string. Print with bounds */
        printf("  [%2d]  %-14d  \"%s\"  (L%d:C%d)\n",
               i,
               tokens[i].id,
               tokens[i].lexeme,
               tokens[i].line,
               tokens[i].col);
    }

    if (tokens_only) return 0;

    /* ── 3. Build grammar structures ─────────────────────────── */
    grammar_init();
    compute_first();
    compute_follow();

    if (!build_parse_table()) {
        fprintf(stderr, "ERROR: Grammar is not LL(1). Aborting.\n");
        return 1;
    }

    /* ── 4. Parse ────────────────────────────────────────────── */
    TokenStream ts;
    ts_init(&ts, tokens, tok_count);

    ParseTreeNode *tree = parser_parse(&ts, argv[1], source);

    /* ── 4.5  Grammar debug (engine data) ───────────────────── */
    printf("\n=== GRAMMAR ANALYSIS ===\n");
    print_first_sets();
    print_follow_sets();
    print_parse_table();
    printf("========================\n\n");

    /* ── 5. Parse result check ───────────────────────────────── */
    if (!tree || error_count > 0) {
        printf("\n=== PARSE FAILED (%d error(s)) ===\n", error_count);
        return 1;
    }

    print_tree(tree, "", true);
    printf("\n=== PARSE SUCCESSFUL ===\n");

    /* ── 6. Semantic analysis ────────────────────────────────── */
    SymbolTable sym_table;
    symbol_table_init(&sym_table);

    int sem_errors = semantic_analyse(tree, &sym_table);

    symbol_print(&sym_table);   /* dump symbol table for demo */

    if (sem_errors > 0) {
        printf("\n=== SEMANTIC FAILED (%d error(s)) ===\n", sem_errors);
        symbol_table_destroy(&sym_table);
        return 1;
    }
    printf("\n=== SEMANTIC OK ===\n");

    /* ── 7. Intermediate Code Generation ────────────────────── */
    InstrList *ilist = icg_list_create();
    icg_gen_stmt(tree, ilist);

    printf("\n=== ICG QUADRUPLES ===\n");
    icg_print_list(ilist);

    icg_list_free(ilist);
    symbol_table_destroy(&sym_table);
    return 0;
}
