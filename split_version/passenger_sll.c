/*
 * passenger_sll.c - SINGLY LINKED LIST of confirmed passengers.
 * passengers -> [100001] -> [100002] -> NULL
 */
#include "railway.h"

/* Adds a passenger at the end of the confirmed list. */
void addPassenger(Train *t, Passenger *p) {
    p->next = NULL;
    if (t->passengers == NULL) { t->passengers = p; return; }
    Passenger *temp = t->passengers;
    while (temp->next != NULL) temp = temp->next;
    temp->next = p;
}

/* Searches a list (confirmed list or waiting queue) for a PNR. */
Passenger *searchPnr(Passenger *head, int pnr) {
    Passenger *temp = head;
    while (temp != NULL) {
        if (temp->pnr == pnr) return temp;
        temp = temp->next;
    }
    return NULL;
}

/* Removes the passenger with this PNR from the confirmed list and returns it. */
Passenger *deletePassenger(Train *t, int pnr) {
    Passenger *temp = t->passengers, *prev = NULL;
    while (temp != NULL && temp->pnr != pnr) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return NULL;          /* not found */
    if (prev == NULL) t->passengers = temp->next;
    else prev->next = temp->next;
    return temp;
}
