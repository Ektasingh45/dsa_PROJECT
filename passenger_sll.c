/*
 * passenger_sll.c
 * ---------------
 * SINGLY LINKED LIST of confirmed passengers of one train.
 *
 *   passengerHead -> [100001 Rahul] -> [100002 Amit] -> NULL
 */
#include "railway.h"

/* Adds a passenger at the end of the list (so the list stays in booking order). */
void addPassenger(struct TrainNode *train, struct Passenger *passenger)
{
    struct Passenger *current;

    passenger->next = NULL;

    if (train->passengerHead == NULL)
    {
        train->passengerHead = passenger;
        return;
    }

    current = train->passengerHead;
    while (current->next != NULL)
        current = current->next;
    current->next = passenger;
}

/* Returns the passenger with this PNR, or NULL. */
struct Passenger *searchPassenger(struct TrainNode *train, int pnr)
{
    struct Passenger *current = train->passengerHead;

    while (current != NULL)
    {
        if (current->pnr == pnr)
            return current;
        current = current->next;
    }
    return NULL;
}

/*
 * Takes the passenger with this PNR out of the list and returns it.
 * The node is NOT freed here - the caller decides what to do with it.
 */
struct Passenger *removePassenger(struct TrainNode *train, int pnr)
{
    struct Passenger *current = train->passengerHead;
    struct Passenger *previous = NULL;

    while (current != NULL && current->pnr != pnr)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
        return NULL;                        /* PNR not found */

    if (previous == NULL)
        train->passengerHead = current->next;   /* removing the first node */
    else
        previous->next = current->next;

    current->next = NULL;
    return current;
}

/* Prints all confirmed passengers as a table. */
void displayPassengers(struct TrainNode *train)
{
    struct Passenger *current = train->passengerHead;

    if (current == NULL)
    {
        printf("No confirmed passengers.\n");
        return;
    }

    printf("%-8s %-18s %-4s %-7s %-6s %-15s %-15s %s\n",
           "PNR", "Name", "Age", "Gender", "Seat", "From", "To", "Fare");
    printf("------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        printf("%-8d %-18.18s %-4d %-7.7s C%d-%02d  %-15.15s %-15.15s Rs. %.0f\n",
               current->pnr, current->name, current->age, current->gender,
               current->coachNumber, current->seatNumber,
               current->source, current->destination, current->fare);
        current = current->next;
    }
}

/* Counts confirmed passengers. */
int countPassengers(struct TrainNode *train)
{
    struct Passenger *current = train->passengerHead;
    int count = 0;

    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}

/* Frees the whole passenger list. */
void freePassengers(struct TrainNode *train)
{
    struct Passenger *current = train->passengerHead;
    struct Passenger *nextPassenger;

    while (current != NULL)
    {
        nextPassenger = current->next;
        free(current);
        current = nextPassenger;
    }
    train->passengerHead = NULL;
}
