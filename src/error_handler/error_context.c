#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error_context.h"

void error_context_build(ErrorContext *ctx, const char *filename, const Token *token, const char *source_code, int err_code) {
    ctx->filename = filename ? strdup(filename) : NULL;
    ctx->line = token->line;
    ctx->col = token->col;
    ctx->err_code = err_code;

    // Extract the source line
    const char *start = source_code;
    const char *end = source_code;
    int current_line = 1;
    while (*end && current_line < token->line) {
        if (*end == '\n') {
            current_line++;
            start = end + 1;
        }
        end++;
    }
    // Now start points to the beginning of the line
    end = start;
    while (*end && *end != '\n') {
        end++;
    }
    // Copy the line
    int len = end - start;
    if (len >= sizeof(ctx->source_line)) {
        len = sizeof(ctx->source_line) - 1;
    }
    strncpy(ctx->source_line, start, len);
    ctx->source_line[len] = '\0';

    // Build caret
    int caret_pos = token->col - 1; // Assuming col is 1-based
    if (caret_pos < 0) caret_pos = 0;
    if (caret_pos >= sizeof(ctx->caret) - 2) caret_pos = sizeof(ctx->caret) - 3;
    memset(ctx->caret, ' ', caret_pos);
    ctx->caret[caret_pos] = '^';
    ctx->caret[caret_pos + 1] = '\0';
}