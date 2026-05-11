#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "type_system.h"

#define HASH_SIZE 211

typedef struct SymbolRecord {
    char name[64];
    TypeNode *type;
    int scope_level;
    int line_declared;
    struct SymbolRecord *next;
} SymbolRecord;

typedef struct ScopeFrame {
    SymbolRecord *buckets[HASH_SIZE];
    struct ScopeFrame *parent;
} ScopeFrame;

typedef struct SymbolTable {
    ScopeFrame *current;
    int depth;
} SymbolTable;

void symbol_table_init(SymbolTable *table);
void symbol_table_destroy(SymbolTable *table);

void scope_enter(SymbolTable *table);
void scope_exit(SymbolTable *table);

unsigned int symbol_hash(const char *name);
SymbolRecord *symbol_lookup_local(const ScopeFrame *frame, const char *name);
SymbolRecord *symbol_lookup(const SymbolTable *table, const char *name);
SymbolRecord *symbol_insert(SymbolTable *table, const char *name, TypeNode *type, int line);

void symbol_print(const SymbolTable *table);

#endif