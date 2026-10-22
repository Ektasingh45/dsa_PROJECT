/*
 * waiting_queue.c
 * ---------------
 * Implementation of the waiting-list QUEUE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "waiting_queue.h"
#include "route_dll.h"

static void copyText(char *dest, const char *src, int size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

void initQueue(WaitingQueue *q)
{
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

int isQueueEmpty(const WaitingQueue *q)
{
    return q->front == NULL;
}

int enqueue(WaitingQueue *q, int pnr, const char *name, int age, const char *gender,
            const char *source, const char *destination, float fare)
{
    WaitingPassenger *node = (WaitingPassenger *)malloc(sizeof(WaitingPassenger));
    if (node == NULL)
    {
        printf("Memory allocation failed for waiting list entry.\n");
        return 0;
    }
    node->pnr = pnr;
    copyText(node->name, name, NAME_LEN);
    node->age = age;
    copyText(node->gender, gender, GENDER_LEN);
    copyText(node->source, source, NAME_LEN);
    copyText(node->destination, destination, NAME_LEN);
    node->fare = fare;
    node->next = NULL;

    if (q->rear == NULL)
        q->front = node;
    else
        q->rear->next = node;
    q->rear = node;
    q->size++;
    return 1;
}

int dequeue(WaitingQueue *q, WaitingPassenger *out)
{
    WaitingPassenger *node;
    if (isQueueEmpty(q))
        return 0;

    node = q->front;
    q->front = node->next;
    if (q->front == NULL)
        q->rear = NULL;
    q->size--;

    if (out != NULL)
    {
        *out = *node;
        out->next = NULL;
    }
    free(node);
    return 1;
}

WaitingPassenger *peekQueue(const WaitingQueue *q)
{
    return q->front;
}

int restoreFront(WaitingQueue *q, const WaitingPassenger *data)
{
    WaitingPassenger *node = (WaitingPassenger *)malloc(sizeof(WaitingPassenger));
    if (node == NULL)
    {
        printf("Memory allocation failed for waiting list entry.\n");
        return 0;
    }
    *node = *data;
    node->next = q->front;
    q->front = node;
    if (q->rear == NULL)
        q->rear = node;
    q->size++;
    return 1;
}

int queuePosition(const WaitingQueue *q, int pnr)
{
    WaitingPassenger *cur = q->front;
    int pos = 1;
    while (cur != NULL)
    {
        if (cur->pnr == pnr)
            return pos;
        pos++;
        cur = cur->next;
    }
    return 0;
}

WaitingPassenger *searchQueue(const WaitingQueue *q, int pnr)
{
    WaitingPassenger *cur = q->front;
    while (cur != NULL)
    {
        if (cur->pnr == pnr)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

int queueUsesStation(const WaitingQueue *q, const char *station)
{
    WaitingPassenger *cur = q->front;
    while (cur != NULL)
    {
        if (namesEqual(cur->source, station) || namesEqual(cur->destination, station))
            return 1;
        cur = cur->next;
    }
    return 0;
}

void displayQueue(const WaitingQueue *q)
{
    WaitingPassenger *cur = q->front;
    int pos = 1;

    if (isQueueEmpty(q))
    {
        printf("Waiting list is empty.\n");
        return;
    }

    printf("Waiting passengers: %d\n\n", q->size);
    printf("FRONT\n  |\n  v\n");
    while (cur != NULL)
    {
        printf("WL%-3d PNR %-7d %-18.18s Age %-3d %-7.7s %s -> %s",
               pos, cur->pnr, cur->name, cur->age, cur->gender,
               cur->source, cur->destination);
        if (cur == q->rear)
            printf("   <- REAR");
        printf("\n");
        pos++;
        cur = cur->next;
    }
}

void freeQueue(WaitingQueue *q)
{
    while (dequeue(q, NULL))
        ;
}
