/*
 * passenger_sll.c
 * ---------------
 * Implementation of the confirmed-passenger SINGLY LINKED LIST.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "passenger_sll.h"
#include "route_dll.h"

static void copyText(char *dest, const char *src, int size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

Passenger *createPassenger(int pnr, const char *name, int age, const char *gender,
                           int trainNumber, int coachNumber, int seatNumber,
                           const char *source, const char *destination, float fare)
{
    Passenger *p = (Passenger *)malloc(sizeof(Passenger));
    if (p == NULL)
    {
        printf("Memory allocation failed for passenger.\n");
        return NULL;
    }
    p->pnr = pnr;
    copyText(p->name, name, NAME_LEN);
    p->age = age;
    copyText(p->gender, gender, GENDER_LEN);
    p->trainNumber = trainNumber;
    p->coachNumber = coachNumber;
    p->seatNumber = seatNumber;
    copyText(p->source, source, NAME_LEN);
    copyText(p->destination, destination, NAME_LEN);
    p->fare = fare;
    p->next = NULL;
    return p;
}

void addPassenger(Passenger **head, Passenger *node)
{
    Passenger *cur;
    if (node == NULL)
        return;
    node->next = NULL;

    if (*head == NULL)
    {
        *head = node;
        return;
    }
    cur = *head;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = node;
}

Passenger *searchPassenger(Passenger *head, int pnr)
{
    while (head != NULL)
    {
        if (head->pnr == pnr)
            return head;
        head = head->next;
    }
    return NULL;
}

Passenger *removePassenger(Passenger **head, int pnr)
{
    Passenger *cur = *head, *prev = NULL;

    while (cur != NULL && cur->pnr != pnr)
    {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL)
        return NULL;

    if (prev == NULL)
        *head = cur->next;
    else
        prev->next = cur->next;
    cur->next = NULL;
    return cur;
}

int deletePassenger(Passenger **head, int pnr)
{
    Passenger *node = removePassenger(head, pnr);
    if (node == NULL)
        return 0;
    free(node);
    return 1;
}

int passengerUsesStation(Passenger *head, const char *station)
{
    while (head != NULL)
    {
        if (namesEqual(head->source, station) || namesEqual(head->destination, station))
            return 1;
        head = head->next;
    }
    return 0;
}

void displayPassengers(Passenger *head)
{
    if (head == NULL)
    {
        printf("No confirmed passengers.\n");
        return;
    }

    printf("%-8s %-18s %-4s %-7s %-6s %-15s %-15s %s\n",
           "PNR", "Name", "Age", "Gender", "Seat", "From", "To", "Fare");
    printf("------------------------------------------------------------------------------------\n");
    while (head != NULL)
    {
        printf("%-8d %-18.18s %-4d %-7.7s C%d-%02d  %-15.15s %-15.15s Rs. %.0f\n",
               head->pnr, head->name, head->age, head->gender,
               head->coachNumber, head->seatNumber,
               head->source, head->destination, head->fare);
        head = head->next;
    }
}

int countPassengers(Passenger *head)
{
    int count = 0;
    while (head != NULL)
    {
        count++;
        head = head->next;
    }
    return count;
}

void freePassengerList(Passenger **head)
{
    Passenger *cur = *head, *next;
    while (cur != NULL)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
    *head = NULL;
}
