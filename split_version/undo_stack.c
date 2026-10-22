/*
 * undo_stack.c - STACK of operations for undo (Last In, First Out).
 * top -> [CANCEL 100005] -> [BOOK 100004] -> NULL
 */
#include "railway.h"

Undo *top = NULL;               /* top of the undo stack */

/* Pushes a booking (type 1) or cancellation (type 2) onto the undo stack. */
void push(int type, int trainNo, Passenger *p, int promotedPnr) {
    Undo *newNode = malloc(sizeof(Undo));
    if (!newNode) return;
    newNode->type = type;
    newNode->trainNo = trainNo;
    newNode->data = *p;                     /* save a copy of the ticket */
    newNode->promotedPnr = promotedPnr;
    newNode->next = top;
    top = newNode;
}

/* Removes and returns the last operation (NULL if the stack is empty). */
Undo *pop() {
    Undo *temp = top;
    if (temp != NULL) top = temp->next;
    return temp;
}
