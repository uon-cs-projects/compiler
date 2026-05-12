#include <stdio.h>
#include "error_handler.h"
#include "../parser/first_follow.h"

#include "tokens.h"
#include "error_context.h"
#include "error_hints.h"

static const char *friendly_nonterminal_name(int nt_idx) {
    switch (nt_idx) {
        case NT_PROGRAM:
        case NT_PROGRAM_TAIL:
            return "program";
        case NT_ELEMENT:
            return "top-level element";
        case NT_FUNCTION:
            return "function definition";
        case NT_PARAM_LIST:
        case NT_PARAM_LIST_TAIL:
            return "parameter list";
        case NT_BLOCK:
            return "block";
        case NT_STMT_LIST:
            return "statement list";
        case NT_STMT:
            return "statement";
        case NT_STMT_ID_TAIL:
            return "assignment or function call";
        case NT_IF_STMT:
        case NT_IF_TAIL:
            return "if statement";
        case NT_WHILE_STMT:
            return "while statement";
        case NT_FOR_STMT:
            return "for statement";
        case NT_PRINT_STMT:
            return "print statement";
        case NT_BREAK_STMT:
            return "break statement";
        case NT_EXPR:
        case NT_EXPR_TAIL:
        case NT_AND_EXPR:
        case NT_AND_EXPR_TAIL:
        case NT_NOT_EXPR:
        case NT_COMP_EXPR:
        case NT_COMP_EXPR_TAIL:
        case NT_MATH_EXPR:
        case NT_MATH_EXPR_TAIL:
        case NT_TERM:
        case NT_TERM_TAIL:
        case NT_FACTOR:
        case NT_FACTOR_ID_TAIL:
            return "expression";
        case NT_ARG_LIST:
        case NT_ARG_LIST_TAIL:
            return "argument list";
        default:
            return "syntax";
    }
}

int error_count = 0;
int warning_count = 0;

void report_error(int line, const char *phase, const char *message) {
    error_count++;
    fprintf(stderr, "\033[1;31m[ %s ERROR ]\033[0m Line %d: %s\n", phase, line, message);
}

void report_warning(int line, const char *phase, const char *message) {
    warning_count++;
    fprintf(stderr, "\033[1;33m[ %s WARNING ]\033[0m Line %d: %s\n", phase, line, message);
}

void finalize_compilation() {
    if (error_count > 0) {
        printf("\nCompilation failed with %d error(s) and %d warning(s).\n", error_count, warning_count);
    } else {
        printf("\nCompilation successful! (%d warnings)\n", warning_count);
    }
}

//parse error handling:
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

void error_match(int expected, const Token *got, const char *filename, const char *source_code) {
    error_count++;
    ErrorContext ctx;
    error_context_build(&ctx, filename, got, source_code, 0); // err_code 0 for match error

    fprintf(stderr, "error: unexpected token '%s' on line %d, column %d\n",
            got->lexeme, ctx.line, ctx.col);
    fprintf(stderr, "%d | %s\n", ctx.line, ctx.source_line);
    fprintf(stderr, "   | %s\n", ctx.caret);
    fprintf(stderr, "Expected: '%s'\n", get_token_name(expected));
    fprintf(stderr, "Hint: %s\n", error_get_hint(ctx.err_code));
    fprintf(stderr, "\n");
}

void error_predict(int nt_idx, const Token *got, const char *filename, const char *source_code) {
    error_count++;
    ErrorContext ctx;
    error_context_build(&ctx, filename, got, source_code, 0); // err_code 0

    fprintf(stderr, "error: unexpected token '%s' on line %d, column %d\n",
            got->lexeme, ctx.line, ctx.col);
    fprintf(stderr, "%d | %s\n", ctx.line, ctx.source_line);
    fprintf(stderr, "   | %s\n", ctx.caret);
    fprintf(stderr, "Unexpected token while parsing %s.\n", friendly_nonterminal_name(nt_idx));
    fprintf(stderr, "Hint: %s\n", error_get_hint(ctx.err_code));
    fprintf(stderr, "\n");
}

Token *error_recover(int nt_idx, TokenStream *ts) {
    while (ts->pos < ts->count) {
        Token *current_tok = &ts->tokens[ts->pos];

        /* If token is in FOLLOW set, we've found a synchronization point */
        if (follow_sets[nt_idx][current_tok->id]) {
            return current_tok;
        }

        /* Skip the token by incrementing the stream position */
        ts->pos++;
    }

    if (ts->count > 0) {
        return &ts->tokens[ts->count - 1];
    }
    return NULL;
}