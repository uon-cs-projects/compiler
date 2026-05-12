#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

typedef enum {
    TK_INT,
    TK_STRING,
    TK_BOOL,
    TK_ARRAY,
    TK_POINTER,
    TK_VOID,
    TK_ERROR
} TypeKind;

typedef struct TypeNode {
    TypeKind kind;
    int array_size;
    struct TypeNode *base_type;
} TypeNode;

TypeNode *type_make_basic(TypeKind kind);
TypeNode *type_make_array(TypeNode *base, int size);
TypeNode *type_make_pointer(TypeNode *base);
int type_equivalent(const TypeNode *t1, const TypeNode *t2);
void type_free(TypeNode *type);

#endif