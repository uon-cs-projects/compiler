#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "../error_handler/error_handler.h"

static void free_record_chain(SymbolRecord *head) {
    SymbolRecord *cur = head;
    while (cur != NULL) {
        SymbolRecord *next = cur->next;
        type_free(cur->type);
        free(cur);
        cur = next;
    }
}

unsigned int symbol_hash(const char *name) {
    unsigned long h = 5381;
    int c;

    while ((c = *name++) != 0) {
        h = ((h << 5) + h) + (unsigned long)c;
    }
    return (unsigned int)(h % HASH_SIZE);
}

void symbol_table_init(SymbolTable *table) {
    table->current = NULL;
    table->depth = -1;
    scope_enter(table);
}

void symbol_table_destroy(SymbolTable *table) {
    while (table->current != NULL) {
        scope_exit(table);
    }
}

void scope_enter(SymbolTable *table) {
    ScopeFrame *frame = (ScopeFrame *)calloc(1, sizeof(ScopeFrame));
    if (frame == NULL) {
        report_error(0, "SEMANTIC", "Failed to allocate scope frame");
        exit(1);
    }

    frame->parent = table->current;
    table->current = frame;
    table->depth++;
}

void scope_exit(SymbolTable *table) {
    ScopeFrame *frame = table->current;
    if (frame == NULL) {
        return;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        free_record_chain(frame->buckets[i]);
    }

    table->current = frame->parent;
    free(frame);
    table->depth--;
}

SymbolRecord *symbol_lookup_local(const ScopeFrame *frame, const char *name) {
    if (frame == NULL) {
        return NULL;
    }

    unsigned int h = symbol_hash(name);
    SymbolRecord *cur = frame->buckets[h];

    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

SymbolRecord *symbol_lookup(const SymbolTable *table, const char *name) {
    const ScopeFrame *frame = table->current;
    while (frame != NULL) {
        SymbolRecord *hit = symbol_lookup_local(frame, name);
        if (hit != NULL) {
            return hit;
        }
        frame = frame->parent;
    }
    return NULL;
}

SymbolRecord *symbol_insert(SymbolTable *table, const char *name, TypeNode *type, int line) {
    if (table->current == NULL) {
        report_error(line, "SEMANTIC", "No active scope for insertion");
        return NULL;
    }

    if (symbol_lookup_local(table->current, name) != NULL) {
        char msg[160];
        snprintf(msg, sizeof(msg), "Redeclaration of identifier '%s'", name);
        report_error(line, "SEMANTIC", msg);
        return NULL;
    }

    SymbolRecord *rec = (SymbolRecord *)calloc(1, sizeof(SymbolRecord));
    if (rec == NULL) {
        report_error(line, "SEMANTIC", "Failed to allocate symbol record");
        exit(1);
    }

    unsigned int h = symbol_hash(name);
    snprintf(rec->name, sizeof(rec->name), "%s", name);
    rec->type = type;
    rec->scope_level = table->depth;
    rec->line_declared = line;
    rec->next = table->current->buckets[h];
    table->current->buckets[h] = rec;

    return rec;
}

void symbol_print(const SymbolTable *table) {
    const ScopeFrame *frame = table->current;
    int level = table->depth;

    printf("\n=== SYMBOL TABLE DUMP ===\n");
    while (frame != NULL) {
        printf("Scope %d:\n", level);
        printf("%-20s %-10s %-8s\n", "NAME", "TYPE", "DECL_LINE");

        for (int i = 0; i < HASH_SIZE; i++) {
            SymbolRecord *cur = frame->buckets[i];
            while (cur != NULL) {
                const char *kind = "UNKNOWN";
                switch (cur->type ? cur->type->kind : TK_ERROR) {
                    case TK_INT: kind = "INT"; break;
                    case TK_FLOAT: kind = "FLOAT"; break;
                    case TK_BOOL: kind = "BOOL"; break;
                    case TK_ARRAY: kind = "ARRAY"; break;
                    case TK_POINTER: kind = "POINTER"; break;
                    case TK_VOID: kind = "VOID"; break;
                    case TK_ERROR: kind = "ERROR"; break;
                }

                printf("%-20s %-10s %-8d\n", cur->name, kind, cur->line_declared);
                cur = cur->next;
            }
        }

        frame = frame->parent;
        level--;
    }
}