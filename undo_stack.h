/*
 * undo_stack.h
 * ------------
 * STACK module - history of reservation operations (linked implementation, LIFO).
 *
 *   TOP
 *    |
 *    v
 *   CANCEL PNR 100005
 *   BOOK   PNR 100004
 *   CANCEL PNR 100003
 *
 * Each node stores a full copy (snapshot) of the ticket, not a pointer to
 * the live passenger node. The live node is freed when a ticket is
 * cancelled, so a snapshot is the only safe way to restore it later.
 * A cancellation that auto-promoted a waiting passenger also stores that
 * passenger, so the undo can send them back to the front of the queue.
 */
#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#ifndef NAME_LEN
#define NAME_LEN 100
#endif
#ifndef GENDER_LEN
#define GENDER_LEN 20
#endif

#define MAX_STACK_DISPLAY 10   /* displayStack shows only the most recent entries */

typedef enum
{
    OP_BOOK = 1,
    OP_CANCEL = 2
} OperationType;

typedef struct TicketRecord
{
    int pnr;
    char name[NAME_LEN];
    int age;
    char gender[GENDER_LEN];
    int trainNumber;
    int coachNumber;
    int seatNumber;
    char source[NAME_LEN];
    char destination[NAME_LEN];
    float fare;
} TicketRecord;

typedef struct Operation
{
    OperationType type;
    TicketRecord ticket;      /* the booked / cancelled ticket          */
    int hasPromotion;         /* 1 if the cancel promoted a WL passenger */
    TicketRecord promoted;    /* valid only when hasPromotion == 1       */
    struct Operation *next;
} Operation;

typedef struct UndoStack
{
    Operation *top;
    int size;
} UndoStack;

void initStack(UndoStack *s);
int isStackEmpty(const UndoStack *s);

/* 1 = pushed, 0 = memory failure. 'promoted' may be NULL. */
int push(UndoStack *s, OperationType type, const TicketRecord *ticket,
         const TicketRecord *promoted);

/* Copies the top into *out (if not NULL) and frees it. 1 = success, 0 = empty. */
int pop(UndoStack *s, Operation *out);

Operation *peek(const UndoStack *s);

const char *operationName(OperationType type);

void displayStack(const UndoStack *s);
void freeStack(UndoStack *s);

#endif
