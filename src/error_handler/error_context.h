#ifndef ERROR_CONTEXT_H
#define ERROR_CONTEXT_H

#include "../parser/shared_types.h"

typedef struct {
    char *filename;
    int line;
    int col;
    char source_line[512];
    char caret[512];
    int err_code;
} ErrorContext;

void error_context_build(ErrorContext *ctx, const char *filename, const Token *token, const char *source_code, int err_code);

#endif /* ERROR_CONTEXT_H */