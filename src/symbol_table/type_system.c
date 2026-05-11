#include <stdlib.h>
#include "type_system.h"

TypeNode *type_make_basic(TypeKind kind) {
    TypeNode *node = (TypeNode *)calloc(1, sizeof(TypeNode));
    if (node == NULL) {
        return NULL;
    }

    node->kind = kind;
    node->array_size = 0;
    node->base_type = NULL;
    return node;
}

TypeNode *type_make_array(TypeNode *base, int size) {
    TypeNode *node = type_make_basic(TK_ARRAY);
    if (node == NULL) {
        return NULL;
    }

    node->array_size = size;
    node->base_type = base;
    return node;
}

TypeNode *type_make_pointer(TypeNode *base) {
    TypeNode *node = type_make_basic(TK_POINTER);
    if (node == NULL) {
        return NULL;
    }

    node->base_type = base;
    return node;
}

int type_equivalent(const TypeNode *t1, const TypeNode *t2) {
    if (t1 == t2) {
        return 1;
    }
    if (t1 == NULL || t2 == NULL) {
        return 0;
    }
    if (t1->kind != t2->kind) {
        return 0;
    }

    if (t1->kind == TK_ARRAY && t1->array_size != t2->array_size) {
        return 0;
    }

    return type_equivalent(t1->base_type, t2->base_type);
}

void type_free(TypeNode *type) {
    if (type == NULL) {
        return;
    }

    type_free(type->base_type);
    free(type);
}