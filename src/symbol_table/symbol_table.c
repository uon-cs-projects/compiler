#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "error_handler.h"  /* The Command Center */

int current_scope = 0;
Symbol* hash_table[TABLE_SIZE];

unsigned int hash(char *str) {
    unsigned int h = 5381;
    int c;
    while ((c = *str++))
        h = ((h << 5) + h) + c; 
    return h % TABLE_SIZE;
}

Symbol* lookup(char *name) {
    unsigned int h = hash(name);
    Symbol *s = hash_table[h];
    while (s != NULL) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

Symbol* lookup_current_scope(char *name) {
    unsigned int h = hash(name);
    Symbol *s = hash_table[h];
    while (s != NULL) {
        if (strcmp(s->name, name) == 0 && s->scope_level == current_scope) {
            return s;
        }
        s = s->next;
    }
    return NULL;
}

/* ADVANCED INSERT: Now using report_error */
Symbol* insert(char *name, int token_class, int line, int scope) {
    /* 1. Redeclaration Check */
    if (lookup_current_scope(name) != NULL) {
        char msg[100];
        sprintf(msg, "Redeclaration of identifier '%s'", name);
        
        // Use the centralized handler instead of raw fprintf
        report_error(line, "SEMANTIC", msg);
        return NULL; 
    }

    /* 2. Allocation with Fatal Error Handling */
    unsigned int h = hash(name);
    Symbol *s = (Symbol*)malloc(sizeof(Symbol));
    if (!s) {
        report_error(line, "CRITICAL", "Memory allocation failed for Symbol Table");
        exit(1); // Fatal OS error
    }

    s->name = strdup(name); 
    s->token_class = token_class;
    s->line_declared = line;
    s->scope_level = scope;
    
    /* 3. Safe Defaults */
    s->data_type = TYPE_NONE;
    s->is_initialized = 0;   
    s->memory_offset = -1;   
    s->size = 0;             
    s->value.i_val = 0;      

    /* 4. Head-Insertion */
    s->next = hash_table[h];
    hash_table[h] = s;

    return s;
}

void print_symbol_table() {
    printf("\n%-15s\t%-8s\t%-8s\t%-12s\t%-10s\t%-5s\n", 
           "IDENTIFIER", "CLASS", "SCOPE", "INITIALIZED", "OFFSET", "LINE");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *s = hash_table[i];
        while (s != NULL) {
            printf("%-15s\t%-8d\t%-8d\t%-12s\t%-10d\t%-5d\n", 
                   s->name, 
                   s->token_class, 
                   s->scope_level,
                   s->is_initialized ? "Yes" : "No",
                   s->memory_offset,
                   s->line_declared);
            s = s->next;
        }
    }
}