/*
 * undo_stack.c
 * ------------
 * Implementation of the undo STACK.
 * The actual "undo" logic (which needs trains, seats and queues) lives in
 * railway.c and uses peek()/pop() from here.
 */
#include <stdio.h>
#include <stdlib.h>
#include "undo_stack.h"

void initStack(UndoStack *s)
{
    s->top = NULL;
    s->size = 0;
}

int isStackEmpty(const UndoStack *s)
{
    return s->top == NULL;
}

int push(UndoStack *s, OperationType type, const TicketRecord *ticket,
         const TicketRecord *promoted)
{
    Operation *node = (Operation *)malloc(sizeof(Operation));
    if (node == NULL)
    {
        printf("Memory allocation failed for undo record.\n");
        return 0;
    }
    node->type = type;
    node->ticket = *ticket;
    node->hasPromotion = (promoted != NULL);
    if (promoted != NULL)
        node->promoted = *promoted;

    node->next = s->top;
    s->top = node;
    s->size++;
    return 1;
}

int pop(UndoStack *s, Operation *out)
{
    Operation *node;
    if (isStackEmpty(s))
        return 0;

    node = s->top;
    s->top = node->next;
    s->size--;

    if (out != NULL)
    {
        *out = *node;
        out->next = NULL;
    }
    free(node);
    return 1;
}

Operation *peek(const UndoStack *s)
{
    return s->top;
}

const char *operationName(OperationType type)
{
    return type == OP_BOOK ? "BOOK" : "CANCEL";
}

void displayStack(const UndoStack *s)
{
    Operation *cur = s->top;
    int shown = 0;
    if (cur == NULL)
    {
        printf("Undo stack is empty.\n");
        return;
    }

    printf("TOP\n |\n v\n");
    while (cur != NULL)
    {
        if (shown == MAX_STACK_DISPLAY)
        {
            printf("... and %d older operation(s)\n", s->size - shown);
            return;
        }
        shown++;
        printf("%-6s PNR %d  (%s, Train %d, C%d-%02d)",
               operationName(cur->type), cur->ticket.pnr, cur->ticket.name,
               cur->ticket.trainNumber, cur->ticket.coachNumber, cur->ticket.seatNumber);
        if (cur->hasPromotion)
            printf("  [promoted PNR %d]", cur->promoted.pnr);
        printf("\n");
        cur = cur->next;
    }
}

void freeStack(UndoStack *s)
{
    while (pop(s, NULL))
        ;
}
