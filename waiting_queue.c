/*
 * waiting_queue.c
 * ---------------
 * QUEUE of waiting passengers of one train (First In, First Out).
 *
 *   waitFront -> [Priya] -> [Arjun] -> [Meera] <- waitRear
 *
 * New passengers join at the REAR, promotion takes them from the FRONT.
 */
#include "railway.h"

/* Returns 1 if nobody is waiting. */
int isQueueEmpty(struct TrainNode *train)
{
    return train->waitFront == NULL;
}

/* Adds a passenger at the rear of the queue. */
void enqueue(struct TrainNode *train, struct Passenger *passenger)
{
    passenger->next = NULL;

    if (train->waitRear == NULL)
        train->waitFront = passenger;       /* queue was empty */
    else
        train->waitRear->next = passenger;

    train->waitRear = passenger;
}

/* Removes and returns the passenger at the front, or NULL if the queue is empty. */
struct Passenger *dequeue(struct TrainNode *train)
{
    struct Passenger *first = train->waitFront;

    if (first == NULL)
        return NULL;

    train->waitFront = first->next;
    if (train->waitFront == NULL)
        train->waitRear = NULL;             /* queue became empty */

    first->next = NULL;
    return first;
}

/* Returns the front passenger without removing it. */
struct Passenger *peekQueue(struct TrainNode *train)
{
    return train->waitFront;
}

/*
 * Puts a passenger back at the FRONT of the queue.
 * Used only by UNDO, to put a promoted passenger back exactly where they were.
 */
void addToFrontOfQueue(struct TrainNode *train, struct Passenger *passenger)
{
    passenger->next = train->waitFront;
    train->waitFront = passenger;

    if (train->waitRear == NULL)
        train->waitRear = passenger;
}

/* Counts waiting passengers. */
int countWaiting(struct TrainNode *train)
{
    struct Passenger *current = train->waitFront;
    int count = 0;

    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}

/* Returns the waiting-list position (1 = front) of a PNR, or 0 if not waiting. */
int getWaitingPosition(struct TrainNode *train, int pnr)
{
    struct Passenger *current = train->waitFront;
    int position = 1;

    while (current != NULL)
    {
        if (current->pnr == pnr)
            return position;
        position++;
        current = current->next;
    }
    return 0;
}

/* Returns the waiting passenger with this PNR, or NULL. */
struct Passenger *searchWaitingList(struct TrainNode *train, int pnr)
{
    struct Passenger *current = train->waitFront;

    while (current != NULL)
    {
        if (current->pnr == pnr)
            return current;
        current = current->next;
    }
    return NULL;
}

/* Prints the queue from FRONT to REAR. */
void displayWaitingList(struct TrainNode *train)
{
    struct Passenger *current = train->waitFront;
    int position = 1;

    if (current == NULL)
    {
        printf("Waiting list is empty.\n");
        return;
    }

    printf("Waiting passengers: %d\n\n", countWaiting(train));
    printf("FRONT\n  |\n  v\n");

    while (current != NULL)
    {
        printf("WL%-3d PNR %-7d %-18.18s Age %-3d %-7.7s %s -> %s",
               position, current->pnr, current->name, current->age,
               current->gender, current->source, current->destination);
        if (current == train->waitRear)
            printf("   <- REAR");
        printf("\n");

        position++;
        current = current->next;
    }
}

/* Frees every waiting passenger. */
void freeWaitingList(struct TrainNode *train)
{
    struct Passenger *passenger = dequeue(train);

    while (passenger != NULL)
    {
        free(passenger);
        passenger = dequeue(train);
    }
}
