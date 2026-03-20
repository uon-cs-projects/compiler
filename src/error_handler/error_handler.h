#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

extern int error_count;
extern int warning_count;

/* Standardized reporting functions */
void report_error(int line, const char *phase, const char *message);
void report_warning(int line, const char *phase, const char *message);

/* The final "Dashboard" summary */
void finalize_compilation();

#endif