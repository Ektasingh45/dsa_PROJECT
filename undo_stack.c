/*
 * undo_stack.c
 * ------------
 * STACK of booking/cancellation operations (Last In, First Out).
 *
 *   undoTop -> [CANCEL 100005] -> [BOOK 100004] -> [BOOK 100003] -> NULL
 *
 * Each node keeps a COPY of the ticket, not a pointer, because a
 * cancelled passenger node is freed and a pointer to it would be invalid.
 */
#include "railway.h"

struct UndoNode *undoTop = NULL;    /* top of the stack */

/* Returns 1 if there is nothing to undo. */
int isStackEmpty(void)
{
    return undoTop == NULL;
}

/* Pushes a new operation. 'promoted' can be NULL if nobody was promoted. */
void pushOperation(int operation, struct Passenger *ticket, struct Passenger *promoted)
{
    struct UndoNode *node = malloc(sizeof(struct UndoNode));
    checkMemory(node);

    node->operation = operation;
    node->ticket = *ticket;                 /* copy the whole structure */
    node->ticket.next = NULL;

    if (promoted != NULL)
    {
        node->hasPromotion = 1;
        node->promoted = *promoted;
        node->promoted.next = NULL;
    }
    else
        node->hasPromotion = 0;

    node->next = undoTop;                   /* new node goes on top */
    undoTop = node;
}

/* Removes and returns the top operation (the caller must free it), or NULL if empty. */
struct UndoNode *popOperation(void)
{
    struct UndoNode *top = undoTop;

    if (top == NULL)
        return NULL;

    undoTop = top->next;
    top->next = NULL;
    return top;
}

/* Returns the top operation without removing it. */
struct UndoNode *peekOperation(void)
{
    return undoTop;
}

/* Prints the 10 most recent operations, newest first. */
void displayStack(void)
{
    struct UndoNode *node = undoTop;
    int shown = 0, remaining = 0;

    if (node == NULL)
    {
        printf("Undo stack is empty.\n");
        return;
    }

    printf("TOP\n |\n v\n");
    while (node != NULL && shown < 10)
    {
        if (node->operation == OP_BOOK)
            printf("BOOK   ");
        else
            printf("CANCEL ");

        printf("PNR %d  (%s, Train %d, C%d-%02d)", node->ticket.pnr, node->ticket.name,
               node->ticket.trainNumber, node->ticket.coachNumber, node->ticket.seatNumber);
        if (node->hasPromotion)
            printf("  [promoted PNR %d]", node->promoted.pnr);
        printf("\n");

        shown++;
        node = node->next;
    }

    while (node != NULL)                    /* count the older ones we did not print */
    {
        remaining++;
        node = node->next;
    }
    if (remaining > 0)
        printf("... and %d older operation(s)\n", remaining);
}

/* Frees the whole stack. */
void freeStack(void)
{
    struct UndoNode *node = popOperation();

    while (node != NULL)
    {
        free(node);
        node = popOperation();
    }
}
