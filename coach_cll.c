/*
 * coach_cll.c - CIRCULAR LINKED LIST of coaches.
 * C1 -> C2 -> C3 -> C4 -> C5 -> back to C1
 */
#include "railway.h"

/* Creates 5 coaches in a circle: C1 -> C2 -> C3 -> C4 -> C5 -> back to C1. */
Coach *createCoaches() {
    Coach *head = NULL, *last = NULL;
    for (int i = 1; i <= 5; i++) {
        Coach *newNode = malloc(sizeof(Coach));
        if (!newNode) exit(1);
        newNode->number = i;
        if (head == NULL) head = newNode;
        else last->next = newNode;
        last = newNode;
    }
    last->next = head;          /* last coach points back to the first */
    return head;
}

/* Moves the train to its next coach. After C5 it comes back to C1. */
void nextCoach(Train *t) {
    t->currentCoach = t->currentCoach->next;
}
