/*
 * coach_cll.h
 * -----------
 * CIRCULAR LINKED LIST module - coaches of one train.
 *
 *     C1 -> C2 -> C3 -> C4 -> C5
 *     ^                       |
 *     +-----------------------+
 *
 * The last coach points back to the first one, so seat allocation can
 * keep rotating through coaches (round-robin) instead of always
 * starting with C1. This spreads passengers evenly across the train.
 */
#ifndef COACH_CLL_H
#define COACH_CLL_H

typedef struct Coach
{
    int coachNumber;
    struct Coach *next;
} Coach;

/* Builds C1..Ccount as a circular list. Returns the head (C1) or NULL on failure. */
Coach *createCoaches(int count);

void displayCoaches(Coach *head);

/* Moves one step around the circle. */
Coach *getNextCoach(Coach *current);

int countCoaches(Coach *head);

void freeCoaches(Coach **head);

#endif
