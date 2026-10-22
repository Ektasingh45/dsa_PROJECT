/*
 * railway.h - the one header shared by all files.
 * It has the structures, the global variables and the list of all functions.
 */
#ifndef RAILWAY_H
#define RAILWAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================= STRUCTURES ======================= */
typedef struct Station {        /* one station of a route (doubly linked list) */
    char name[50];
    struct Station *prev, *next;
} Station;

typedef struct Coach {          /* one coach (circular linked list) */
    int number;
    struct Coach *next;
} Coach;

typedef struct Passenger {      /* one ticket (used in the passenger list AND the waiting queue) */
    int pnr, age, fare;
    char name[50], from[50], to[50];
    int coach, seat;            /* coach 0 = no seat yet (waiting) */
    struct Passenger *next;
} Passenger;

typedef struct Train {          /* one train = one node of the BST */
    int number;
    char name[50];
    Station *head, *tail;       /* route: doubly linked list */
    int seats[5][10];           /* seats: 2D array, 0 = free, 1 = booked */
    Coach *currentCoach;        /* coaches: circular linked list */
    Passenger *passengers;      /* confirmed passengers: singly linked list */
    Passenger *front, *rear;    /* waiting list: queue */
    struct Train *left, *right;
} Train;

typedef struct Undo {           /* one saved operation = one node of the stack */
    int type;                   /* 1 = booking, 2 = cancellation */
    int trainNo;
    Passenger data;             /* copy of the ticket */
    int promotedPnr;            /* waiting passenger who got the freed seat (0 = none) */
    struct Undo *next;
} Undo;

/* ======================= GLOBAL VARIABLES ======================= */
extern Train *root;             /* root of the BST          (railway.c)    */
extern int nextPnr;             /* next PNR to give out     (railway.c)    */
extern Undo *top;               /* top of the undo stack    (undo_stack.c) */

/* ======================= FUNCTIONS ======================= */

/* coach_cll.c - Circular Linked List */
Coach *createCoaches();
void nextCoach(Train *t);

/* seat_array.c - 2D Array */
int countFreeSeats(Train *t);
void showSeats(Train *t);
int allotSeat(Train *t, Passenger *p);

/* route_dll.c - Doubly Linked List */
int sameName(char a[], char b[]);
void addStation(Train *t, char name[]);
void displayForward(Train *t);
void displayBackward(Train *t);
int stationIndex(Train *t, char name[]);

/* passenger_sll.c - Singly Linked List */
void addPassenger(Train *t, Passenger *p);
Passenger *searchPnr(Passenger *head, int pnr);
Passenger *deletePassenger(Train *t, int pnr);

/* waiting_queue.c - Queue */
void enqueue(Train *t, Passenger *p);
Passenger *dequeue(Train *t);
void sendBackToQueue(Train *t, int pnr);

/* undo_stack.c - Stack */
void push(int type, int trainNo, Passenger *p, int promotedPnr);
Undo *pop();

/* train_bst.c - Binary Search Tree */
Train *createTrain(int number, char name[]);
Train *insertTrain(Train *node, Train *t);
Train *searchTrain(Train *node, int number);
void displayTrains(Train *node);
Train *findTrainOfPnr(Train *node, int pnr);

/* railway.c - admin and passenger operations */
int readNumber();
void readText(char text[]);
Train *askTrain();
void printTicket(Train *t, Passenger *p);
int promoteWaiting(Train *t);
Passenger *readPassenger(Train *t);
void showAllTrains();
void addTrain();
void addStationMenu();
void bookOnTrain(Train *t);
void bookTicket();
int afterViewMenu(Train *t);
int viewRoute();
int checkSeats();
void cancelTicket();
void viewTicket();
void undoBooking(Train *t, Undo *u);
void undoCancel(Train *t, Undo *u);
void undoLast();

/* main.c - start-up and menus */
void fillTrain(Train *t);
void loadTrains();
void adminMenu();
void passengerMenu();

#endif
