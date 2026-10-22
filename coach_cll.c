/*
 * coach_cll.c
 * -----------
 * CIRCULAR LINKED LIST of coaches.
 *
 *   C1 -> C2 -> C3 -> C4 -> C5
 *   ^                       |
 *   +-----------------------+
 *
 * The last coach points back to the first one, so booking can keep
 * moving to the next coach forever: C1, C2, C3, C4, C5, C1, ...
 */
#include "railway.h"

/* Creates coaches C1..C5 and joins the last coach back to the first. */
void createCoaches(struct TrainNode *train)
{
    struct Coach *newCoach;
    struct Coach *lastCoach = NULL;
    int i;

    train->coachHead = NULL;

    for (i = 1; i <= MAX_COACHES; i++)
    {
        newCoach = malloc(sizeof(struct Coach));
        checkMemory(newCoach);
        newCoach->coachNumber = i;
        newCoach->next = NULL;

        if (train->coachHead == NULL)
            train->coachHead = newCoach;
        else
            lastCoach->next = newCoach;
        lastCoach = newCoach;
    }

    lastCoach->next = train->coachHead;     /* this makes the list circular */
    train->currentCoach = train->coachHead; /* first booking starts at C1   */
}

/* Returns the coach after this one (after C5 comes C1 again). */
struct Coach *getNextCoach(struct Coach *coach)
{
    return coach->next;
}

/* Prints the coaches once around the circle. */
void displayCoaches(struct TrainNode *train)
{
    struct Coach *coach = train->coachHead;

    do
    {
        printf("C%d -> ", coach->coachNumber);
        coach = coach->next;
    } while (coach != train->coachHead);    /* stop when we are back at C1 */

    printf("(back to C%d)\n", train->coachHead->coachNumber);
}

/* Frees all coaches. */
void freeCoaches(struct TrainNode *train)
{
    struct Coach *coach, *nextCoach;

    if (train->coachHead == NULL)
        return;

    coach = train->coachHead->next;
    while (coach != train->coachHead)       /* free C2..C5 */
    {
        nextCoach = coach->next;
        free(coach);
        coach = nextCoach;
    }
    free(train->coachHead);                 /* free C1 */

    train->coachHead = NULL;
    train->currentCoach = NULL;
}
