

#include <stdio.h>
#include <stdlib.h>
#include "double_link_list.h"

inline void list_link(node_t* node, node_t* front)
{
    node->next = front;
    node->prev = front->prev;

    if (front->prev != NULL)    front->prev->next = node;

    front->prev = node;
}

inline void list_unlink(node_t* node)
{
    if (node->prev != NULL)    node->prev->next = node->next;
    if (node->next != NULL)    node->next->prev = node->prev;
}
