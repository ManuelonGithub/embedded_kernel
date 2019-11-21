/**
 * @file    dlist.c
 * @brief   Contains all functionality on how to manage double linked lists.
 * @author  Manuel Burnay
 * @date    2019.11.13 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#include <stdio.h>
#include <stdlib.h>
#include "dlist.h"

/**
 * @brief   Links a node into the back of another.
 * @param   [in,out] node: Node to be linked.
 * @param   [in,out] front: "Head node" where the above node will be linked to.
 */
inline void dLink(node_t* node, node_t* front)
{
    // Making the node connections to the list
    node->next = front;
    node->prev = front->prev;

    // Breaking link between the previous node and the front node.
    if (front->prev != NULL)    front->prev->next = node;

    // Inserting node into the list.
    front->prev = node;
}

/**
 * @brief   Unlinks a node from its current list.
 * @param   [in,out] node: Node to unlink from its list.
 */
inline void dUnlink(node_t* node)
{
    // Sever the connections between the node and its list
    if (node->prev != NULL)    node->prev->next = node->next;
    if (node->next != NULL)    node->next->prev = node->prev;

    // Removing the node's linkage to the list
    node->next = NULL;
    node->prev = NULL;
}
