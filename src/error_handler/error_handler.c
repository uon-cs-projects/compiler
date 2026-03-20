#include <stdio.h>
#include "error_handler.h"

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