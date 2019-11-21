/**
 * @file    dlist.h
 * @brief   Contains all structures and function declarations
 *          on how to manage double linked lists.
 * @author  Manuel Burnay
 * @date    2019.11.13 (Created)
 * @date    2019.11.21 (Last Modified)
 */

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
