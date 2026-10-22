/*
 * railway.h
 * ---------
 * The ONE header file of the project.
 * It contains all constants, all structures and the declarations of all
 * functions, so every .c file only needs:   #include "railway.h"
 */
#ifndef RAILWAY_H
#define RAILWAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------ Constants ------------------------------ */

#define NAME_LEN          100     /* max length of names (99 chars + '\0') */
#define MAX_COACHES       5       /* coaches C1..C5                        */
#define SEATS_PER_COACH   10      /* seats 01..10 in every coach           */
#define TOTAL_SEATS       50      /* 5 x 10                                */

#define SEAT_AVAILABLE    0
#define SEAT_BOOKED       1

#define FIRST_PNR         100001
#define BASE_FARE         250     /* Rs. 250 for every ticket              */
#define FARE_PER_STOP     240     /* + Rs. 240 for every station travelled */

#define MIN_AGE           1
#define MAX_AGE           120

#define OP_BOOK           1       /* operation types stored on the undo stack */
#define OP_CANCEL         2

/* ------------------------------ Structures ----------------------------- */

/* DOUBLY LINKED LIST node: one station on a train's route */
struct Station
{
    char name[NAME_LEN];
    struct Station *prev;
    struct Station *next;
};

/* CIRCULAR LINKED LIST node: one coach of a train */
struct Coach
{
    int coachNumber;
    struct Coach *next;
};

/*
 * One passenger ticket.
 * The SAME structure is used for:
 *   - confirmed passengers (SINGLY LINKED LIST)  -> coachNumber 1..5
 *   - waiting passengers   (QUEUE)               -> coachNumber 0 (no seat yet)
 * so a waiting passenger can simply be moved from the queue to the list.
 */
struct Passenger
{
    int pnr;
    char name[NAME_LEN];
    int age;
    char gender[20];

    int trainNumber;
    int coachNumber;
    int seatNumber;

    char source[NAME_LEN];
    char destination[NAME_LEN];

    float fare;

    struct Passenger *next;
};

/* BINARY SEARCH TREE node: one train (key = trainNumber) with all its own data */
struct TrainNode
{
    int trainNumber;
    char trainName[NAME_LEN];
    char source[NAME_LEN];
    char destination[NAME_LEN];

    struct Station *routeHead;                  /* Doubly linked list: first station   */
    struct Station *routeTail;                  /* Doubly linked list: last station    */

    struct Coach *coachHead;                    /* Circular linked list: coach C1      */
    struct Coach *currentCoach;                 /* coach where the next booking starts */

    int seats[MAX_COACHES][SEATS_PER_COACH];    /* Array: 0 = available, 1 = booked    */

    struct Passenger *passengerHead;            /* Singly linked list: confirmed       */

    struct Passenger *waitFront;                /* Queue: first waiting passenger      */
    struct Passenger *waitRear;                 /* Queue: last waiting passenger       */

    struct TrainNode *left;
    struct TrainNode *right;
};

/* STACK node: one booking or cancellation that can be undone */
struct UndoNode
{
    int operation;                  /* OP_BOOK or OP_CANCEL                          */
    struct Passenger ticket;        /* copy of the booked / cancelled ticket         */
    int hasPromotion;               /* 1 if the cancel promoted a waiting passenger  */
    struct Passenger promoted;      /* copy of that promoted passenger               */
    struct UndoNode *next;
};

/* --------------------------- Global variables -------------------------- */

extern struct TrainNode *trainRoot;   /* root of the BST         (railway.c)    */
extern struct UndoNode *undoTop;      /* top of the undo stack   (undo_stack.c) */

/* ------------------------ train_bst.c  (BST) --------------------------- */

struct TrainNode *createTrain(int trainNumber, char name[], char source[], char destination[]);
struct TrainNode *insertTrain(struct TrainNode *root, struct TrainNode *newTrain);
struct TrainNode *searchTrain(struct TrainNode *root, int trainNumber);
struct TrainNode *findMinimum(struct TrainNode *root);
struct TrainNode *removeMinimum(struct TrainNode *root);
struct TrainNode *deleteTrain(struct TrainNode *root, int trainNumber);
void copyTrainData(struct TrainNode *destination, struct TrainNode *source);
void displayAllTrains(struct TrainNode *root);
int countTrains(struct TrainNode *root);
void freeTrainData(struct TrainNode *train);
void freeTrain(struct TrainNode *train);
void freeAllTrains(struct TrainNode *root);

/* --------------------- route_dll.c  (Doubly Linked List) --------------- */

int sameName(char first[], char second[]);
struct Station *searchStation(struct TrainNode *train, char name[]);
int getStationPosition(struct TrainNode *train, char name[]);
int countStations(struct TrainNode *train);
void addStationAtEnd(struct TrainNode *train, char name[]);
void insertStationAfter(struct TrainNode *train, struct Station *position, char name[]);
void addStationBeforeDestination(struct TrainNode *train, char name[]);
void deleteStation(struct TrainNode *train, struct Station *station);
void displayRouteForward(struct TrainNode *train);
void displayRouteBackward(struct TrainNode *train);
void displayRouteInLine(struct TrainNode *train);
void freeRoute(struct TrainNode *train);

/* -------------------- coach_cll.c  (Circular Linked List) -------------- */

void createCoaches(struct TrainNode *train);
struct Coach *getNextCoach(struct Coach *coach);
void displayCoaches(struct TrainNode *train);
void freeCoaches(struct TrainNode *train);

/* ------------------------ seat_array.c  (Array) ------------------------ */

void initSeats(struct TrainNode *train);
void displaySeats(struct TrainNode *train);
int countFreeSeatsInCoach(struct TrainNode *train, int coachNumber);
int countFreeSeats(struct TrainNode *train);
int findFreeSeat(struct TrainNode *train, int coachNumber);
int isSeatFree(struct TrainNode *train, int coachNumber, int seatNumber);
void bookSeat(struct TrainNode *train, int coachNumber, int seatNumber);
void releaseSeat(struct TrainNode *train, int coachNumber, int seatNumber);

/* ----------------- passenger_sll.c  (Singly Linked List) --------------- */

void addPassenger(struct TrainNode *train, struct Passenger *passenger);
struct Passenger *searchPassenger(struct TrainNode *train, int pnr);
struct Passenger *removePassenger(struct TrainNode *train, int pnr);
void displayPassengers(struct TrainNode *train);
int countPassengers(struct TrainNode *train);
void freePassengers(struct TrainNode *train);

/* ---------------------- waiting_queue.c  (Queue) ----------------------- */

int isQueueEmpty(struct TrainNode *train);
void enqueue(struct TrainNode *train, struct Passenger *passenger);
struct Passenger *dequeue(struct TrainNode *train);
struct Passenger *peekQueue(struct TrainNode *train);
void addToFrontOfQueue(struct TrainNode *train, struct Passenger *passenger);
int countWaiting(struct TrainNode *train);
int getWaitingPosition(struct TrainNode *train, int pnr);
struct Passenger *searchWaitingList(struct TrainNode *train, int pnr);
void displayWaitingList(struct TrainNode *train);
void freeWaitingList(struct TrainNode *train);

/* ----------------------- undo_stack.c  (Stack) ------------------------- */

int isStackEmpty(void);
void pushOperation(int operation, struct Passenger *ticket, struct Passenger *promoted);
struct UndoNode *popOperation(void);
struct UndoNode *peekOperation(void);
void displayStack(void);
void freeStack(void);

/* ------------------- railway.c  (admin + passenger work) --------------- */

/* helpers */
void checkMemory(void *pointer);
void freeAllData(void);
void stopOnEndOfInput(void);
void clearInputBuffer(void);
int readInt(int *value);
void readText(char text[]);
int readYesNo(void);
void printLine(void);
void printHeader(char title[]);
void printTrainDetails(struct TrainNode *train);
void printTicket(struct TrainNode *train, struct Passenger *passenger);
struct TrainNode *askForTrain(void);
float calculateFare(struct TrainNode *train, char source[], char destination[]);
int allocateSeat(struct TrainNode *train, int *coachNumber, int *seatNumber);
struct Passenger *promoteWaitingPassenger(struct TrainNode *train);
struct TrainNode *findTrainOfPassenger(struct TrainNode *root, int pnr);
struct TrainNode *findTrainOfWaitingPassenger(struct TrainNode *root, int pnr);
int isStationInUse(struct TrainNode *train, char name[]);

/* admin */
int adminLogin(void);
void addTrain(void);
void removeTrain(void);
void addStation(void);
void removeStation(void);
void showRouteMenu(void);

/* shared by admin and passenger */
void showTrain(void);
void showAllTrains(void);
void viewRoute(void);
void showSeatStatus(void);

/* passenger */
void readPassengerDetails(struct Passenger *passenger);
struct Station *askStation(struct TrainNode *train, char prompt[]);
int askJourney(struct TrainNode *train, struct Passenger *passenger);
void joinWaitingList(struct TrainNode *train, struct Passenger *passenger);
void bookTicket(void);
void cancelTicket(void);
void viewTicket(void);
void viewWaitingList(void);
void sendBackToWaitingList(struct TrainNode *train, int pnr);
void undoBooking(struct TrainNode *train, struct UndoNode *record);
void undoCancellation(struct TrainNode *train, struct UndoNode *record);
void undoOperation(void);

/* start-up */
void loadInitialData(void);

/* main.c */
void adminMenu(void);
void passengerMenu(void);

#endif
