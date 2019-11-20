

#ifndef DLIST_H
#define DLIST_H

#include <stdint.h>

typedef struct node_ {
    struct node_* next;
    struct node_* prev;
} node_t;

inline void dLink(node_t* node, node_t* front);
inline void dUnlink(node_t* node);

#endif // DLIST_H
