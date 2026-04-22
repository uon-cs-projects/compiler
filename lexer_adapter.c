/* lexer_adapter.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer_adapter.h"
#include "src/lexer/tokens.h"

extern int yylex();
extern char *yytext;
extern int yylineno;

/* Flex functions to explicitly control string buffer parsing */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * yy_str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);

int lexer_adapter_run(const char *source_code, Token *out_tokens, int max_tokens) {
    if (!source_code || !out_tokens || max_tokens <= 0) {
        return 0;
    }

    /* Initialize the lexer buffer to read from the string in memory instead of stdin file */
    YY_BUFFER_STATE buffer = yy_scan_string(source_code);

    int count = 0;
    int token_id;
    
    yylineno = 1; /* Reset line number if multiple passes */

    while ((token_id = yylex()) != 0) {
        if (count >= max_tokens - 1) { 
            fprintf(stderr, "LEXER_ADAPTER: Exceeded MAX_TOKENS limit.\n");
            break;
        }

        out_tokens[count].id = token_id;
        /* strdup yytext so it's not overwritten on next run */
        out_tokens[count].lexeme = yytext ? strdup(yytext) : strdup("");
        out_tokens[count].line = yylineno;
        out_tokens[count].col = 0; // The lexer doesn't track this.
        count++;
    }

    /* Assign EOF token at the end explicitly for the parser to consume */
    out_tokens[count].id = TOK_EOF;
    out_tokens[count].lexeme = strdup("EOF");
    out_tokens[count].line = yylineno;
    out_tokens[count].col = 0;
    count++;

    yy_delete_buffer(buffer);

    return count;
}
