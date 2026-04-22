#include "error_handler.h"
#include "first_follow.h"
#include "tokens.h"
#include <stdio.h>

int error_count = 0;

/* Helper to convert your T_ defines into readable strings */
const char* get_token_name(int token_id) {
    switch (token_id) {
        case T_DEF:     return "def";
        case T_IF:      return "if";
        case T_ELSE:    return "else";
        case T_ELSEIF:  return "else if";
        case T_WHILE:   return "while";
        case T_FOR:     return "for";
        case T_RETURN:  return "return";
        case T_PRINT:   return "print";
        case T_BREAK:   return "break";
        case T_AND:     return "and";
        case T_OR:      return "or";
        case T_NOT:     return "not";
        case T_ID:      return "identifier";
        case T_INT:     return "integer";
        case T_STR:     return "string";
        case T_BOOL:    return "boolean";
        case T_PLUS:    return "+";
        case T_MINUS:   return "-";
        case T_MULT:    return "*";
        case T_DIV:     return "/";
        case T_EQ:      return "==";
        case T_NEQ:     return "!=";
        case T_GT:      return ">";
        case T_LT:      return "<";
        case T_GE:      return ">=";
        case T_LE:      return "<=";
        case T_ASSIGN:  return "=";
        case T_LPAREN:  return "(";
        case T_RPAREN:  return ")";
        case T_LBRACE:  return "{";
        case T_RBRACE:  return "}";
        case T_COMMA:   return ",";
        case T_LBRACKET: return "[";
        case T_RBRACKET: return "]";
        case 0:         return "EOF";
        default:        return "UNKNOWN";
    }
}

void error_match(int expected, const Token *got) {
    error_count++;
    fprintf(stderr, 
        "\n[Syntax Error] Line %d:\n"
        "  Expected: '%s'\n"
        "  Found:    '%s' (\"%s\")\n",
        got->line, 
        get_token_name(expected), 
        get_token_name(got->id), 
        got->lexeme);
}

void error_predict(int nt_idx, const Token *got) {
    error_count++;
    fprintf(stderr,
        "\n[Syntax Error] Line %d:\n"
        "  Unexpected token '%s' (\"%s\") while parsing %s.\n",
        got->line,
        get_token_name(got->id), got->lexeme,
        NONTERMINAL_NAMES[nt_idx]);
}

Token *error_recover(int nt_idx, TokenStream *ts) {
    /* Get current token based on the stream position */
    Token *current_tok = &ts->tokens[ts->pos];
    
    fprintf(stderr, "  Panic Mode: Skipping tokens until FOLLOW(%s)...\n", 
            NONTERMINAL_NAMES[nt_idx]);

    /* * Loop until we find a token in the FOLLOW set of the current non-terminal,
     * or we run out of tokens in the stream.
     */
    while (ts->pos < ts->count) {
        current_tok = &ts->tokens[ts->pos];

        /* If token is in FOLLOW set, we've found a synchronization point */
        if (follow_sets[nt_idx][current_tok->id]) {
            fprintf(stderr, "  Recovered at: '%s' (Line %d)\n", 
                    current_tok->lexeme, current_tok->line);
            return current_tok;
        }

        /* Skip the token by incrementing the stream position */
        ts->pos++;
    }

    return &ts->tokens[ts->pos]; // Returns the last token (usually EOF)
}