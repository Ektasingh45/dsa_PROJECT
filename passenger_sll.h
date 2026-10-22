/*
 * passenger_sll.h
 * ---------------
 * SINGLY LINKED LIST module - confirmed passengers of one train.
 *
 *   head -> [100001 Rahul] -> [100002 Priya] -> [100003 Arjun] -> NULL
 *
 * New bookings are appended at the tail so the list stays in booking order.
 */
#ifndef PASSENGER_SLL_H
#define PASSENGER_SLL_H

#ifndef NAME_LEN
#define NAME_LEN 100
#endif
#define GENDER_LEN 20

typedef struct Passenger
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

    struct Passenger *next;
} Passenger;

/* Allocates and fills a node. Returns NULL if malloc fails. */
Passenger *createPassenger(int pnr, const char *name, int age, const char *gender,
                           int trainNumber, int coachNumber, int seatNumber,
                           const char *source, const char *destination, float fare);

void addPassenger(Passenger **head, Passenger *node);

Passenger *searchPassenger(Passenger *head, int pnr);

/* Unlinks the node with this PNR and returns it (caller must free), NULL if absent. */
Passenger *removePassenger(Passenger **head, int pnr);

/* Unlinks and frees. 1 = deleted, 0 = not found. */
int deletePassenger(Passenger **head, int pnr);

/* 1 if any passenger boards or leaves at this station. */
int passengerUsesStation(Passenger *head, const char *station);

void displayPassengers(Passenger *head);
int countPassengers(Passenger *head);
void freePassengerList(Passenger **head);

#endif
