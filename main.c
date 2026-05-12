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

    if (tree) {
        print_tree(tree, "", true);
        printf("\n=== PARSE SUCCESSFUL ===\n");
    } else {
        printf("\n=== PARSE FAILED ===\n");
    }

    printf("\n=== GRAMMAR ANALYSIS ===\n");
    print_first_sets();
    print_follow_sets();
    print_parse_table();
    printf("========================\n\n");

    /* ── 5. Semantic analysis ─────────────────────────────────
     * Always runs when a parse tree exists — collects ALL type and
     * declaration errors even when lexical/parse errors already exist. */
    SymbolTable sym_table;
    symbol_table_init(&sym_table);

    if (tree) {
        semantic_analyse(tree, &sym_table);
        symbol_print(&sym_table);   /* dump symbol table for demo */
    }

    /* ── 6. ICG gate ────────────────────────────────────────────
     * error_count accumulates ALL errors across every phase above
     * (lexical, parse, semantic). ICG only runs when it is zero.  */
    if (error_count > 0) {
        printf("\n=== COMPILATION FAILED: %d error(s) — ICG skipped ===\n",
               error_count);
        symbol_table_destroy(&sym_table);
        return 1;
    }

    /* ── 7. Intermediate Code Generation ────────────────────── */
    printf("\n=== SEMANTIC OK ===\n");
    InstrList *ilist = icg_list_create();
    icg_gen_stmt(tree, ilist);

    printf("\n=== ICG QUADRUPLES ===\n");
    icg_print_list(ilist);

    icg_list_free(ilist);
    symbol_table_destroy(&sym_table);
    return 0;
}
