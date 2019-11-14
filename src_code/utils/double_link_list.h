

#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <stdint.h>

typedef struct node_ {
    struct node_* next;
    struct node_* prev;
} node_t;

inline void list_link(node_t* node, node_t* front);
inline void list_unlink(node_t* node);

#endif // DOUBLE_LINKED_LIST_H
