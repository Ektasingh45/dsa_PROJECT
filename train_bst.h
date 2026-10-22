/*
 * train_bst.h
 * -----------
 * BINARY SEARCH TREE module - all trains, keyed by train number.
 *
 *               12951
 *              /     \
 *          12431     12952
 *               \
 *               12627
 *
 * Every TrainNode is also the owner of that train's private data
 * structures (route DLL, coach CLL, seat array, passenger SLL and
 * waiting queue), so no two trains ever share reservation data.
 */
#ifndef TRAIN_BST_H
#define TRAIN_BST_H

#include "route_dll.h"
#include "coach_cll.h"
#include "seat_array.h"
#include "passenger_sll.h"
#include "waiting_queue.h"

typedef struct TrainNode
{
    int trainNumber;
    char trainName[NAME_LEN];
    char source[NAME_LEN];
    char destination[NAME_LEN];

    Route route;                                  /* Doubly Linked List   */
    Coach *coaches;                               /* Circular Linked List */
    Coach *nextCoach;                             /* rotation pointer     */
    int seats[MAX_COACHES][SEATS_PER_COACH];      /* Array                */
    Passenger *passengers;                        /* Singly Linked List   */
    WaitingQueue waitingList;                     /* Queue                */

    struct TrainNode *left;
    struct TrainNode *right;
} TrainNode;

/* Creates a train with route (source <-> destination), coaches, seats,
   an empty passenger list and an empty waiting queue. NULL on failure. */
TrainNode *createTrain(int trainNumber, const char *name,
                       const char *source, const char *destination);

/* *inserted = 1 on success, 0 if the train number already exists. */
TrainNode *insertTrain(TrainNode *root, TrainNode *node, int *inserted);

TrainNode *searchTrain(TrainNode *root, int trainNumber);

/* Removes the node and frees ALL of its data. *deleted = 1 if found. */
TrainNode *deleteTrain(TrainNode *root, int trainNumber, int *deleted);

void inorderTrains(TrainNode *root);
int countTrains(TrainNode *root);

void freeTrain(TrainNode *train);
void freeAllTrains(TrainNode *root);

#endif
