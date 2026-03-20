#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum { 
    TYPE_INT, 
    TYPE_STR, 
    TYPE_BOOL, 
    TYPE_FUNC, 
    TYPE_NONE 
} DataType;

typedef struct Symbol {
    char *name;             
    int token_class;        
    DataType data_type;     
    
    union {                
        int i_val;
        char *s_val;
        int b_val;
    } value;

    int scope_level;        
    int is_initialized;     

    int memory_offset;      /* Relative to $sp$ */
    int size;               
    
    int line_declared;
    struct Symbol *next;    
} Symbol;

#define TABLE_SIZE 211 

/* 1. Added the Scope Tracker as an 'extern' 
   This lets the Lexer and Parser see the current 'room' number. */
extern int current_scope; 

unsigned int hash(char *str);
Symbol* lookup(char *name);

/* 2. Added the "Room-Specific" Lookup 
   This is the 'Secret Sauce' that catches redeclaration errors. */
Symbol* lookup_current_scope(char *name);

/* We can keep 'scope' as a parameter for flexibility, 
   or just use the global 'current_scope' inside the function. */
Symbol* insert(char *name, int token_class, int line, int scope);

void print_symbol_table();

#endif