/*
 * waiting_queue.c - QUEUE of waiting passengers (First In, First Out).
 * front -> [Priya] -> [Arjun] <- rear
 */
#include "railway.h"

/* Adds a passenger at the rear of the waiting queue. */
void enqueue(Train *t, Passenger *p) {
    p->next = NULL;
    if (t->rear == NULL) t->front = p;
    else t->rear->next = p;
    t->rear = p;
}

/* Removes and returns the passenger at the front of the waiting queue. */
Passenger *dequeue(Train *t) {
    Passenger *temp = t->front;
    if (temp == NULL) return NULL;
    t->front = temp->next;
    if (t->front == NULL) t->rear = NULL;
    temp->next = NULL;
    return temp;
}

/* Undo helper: takes a promoted passenger out of the confirmed list
   and puts them back at the FRONT of the waiting queue. */
void sendBackToQueue(Train *t, int pnr) {
    Passenger *p = deletePassenger(t, pnr);
    if (p == NULL) return;
    t->seats[p->coach - 1][p->seat - 1] = 0;
    p->coach = 0;
    p->seat = 0;
    p->next = t->front;
    t->front = p;
    if (t->rear == NULL) t->rear = p;
}
