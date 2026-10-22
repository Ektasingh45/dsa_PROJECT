/*
 * train_bst.c
 * -----------
 * Implementation of the train BINARY SEARCH TREE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "train_bst.h"

static void copyText(char *dest, const char *src, int size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

TrainNode *createTrain(int trainNumber, const char *name,
                       const char *source, const char *destination)
{
    TrainNode *t = (TrainNode *)malloc(sizeof(TrainNode));
    if (t == NULL)
    {
        printf("Memory allocation failed for train.\n");
        return NULL;
    }

    t->trainNumber = trainNumber;
    copyText(t->trainName, name, NAME_LEN);
    copyText(t->source, source, NAME_LEN);
    copyText(t->destination, destination, NAME_LEN);
    t->left = NULL;
    t->right = NULL;

    /* 1. Route (DLL): source <-> destination */
    initRoute(&t->route);
    if (addStationAtEnd(&t->route, source) != ROUTE_OK ||
        addStationAtEnd(&t->route, destination) != ROUTE_OK)
    {
        freeRoute(&t->route);
        free(t);
        return NULL;
    }

    /* 2. Coaches (CLL) */
    t->coaches = createCoaches(MAX_COACHES);
    if (t->coaches == NULL)
    {
        freeRoute(&t->route);
        free(t);
        return NULL;
    }
    t->nextCoach = t->coaches;

    /* 3. Seats (Array) */
    initSeats(t->seats);

    /* 4. Confirmed passengers (SLL) and 5. waiting list (Queue) */
    t->passengers = NULL;
    initQueue(&t->waitingList);
    return t;
}

TrainNode *insertTrain(TrainNode *root, TrainNode *node, int *inserted)
{
    if (root == NULL)
    {
        *inserted = 1;
        return node;
    }
    if (node->trainNumber < root->trainNumber)
        root->left = insertTrain(root->left, node, inserted);
    else if (node->trainNumber > root->trainNumber)
        root->right = insertTrain(root->right, node, inserted);
    else
        *inserted = 0; /* duplicate key */
    return root;
}

TrainNode *searchTrain(TrainNode *root, int trainNumber)
{
    while (root != NULL)
    {
        if (trainNumber == root->trainNumber)
            return root;
        root = (trainNumber < root->trainNumber) ? root->left : root->right;
    }
    return NULL;
}

TrainNode *deleteTrain(TrainNode *root, int trainNumber, int *deleted)
{
    TrainNode *replacement, *parent, *succ;

    if (root == NULL)
        return NULL;

    if (trainNumber < root->trainNumber)
    {
        root->left = deleteTrain(root->left, trainNumber, deleted);
        return root;
    }
    if (trainNumber > root->trainNumber)
    {
        root->right = deleteTrain(root->right, trainNumber, deleted);
        return root;
    }

    /* Found the node to delete */
    if (root->left == NULL)
        replacement = root->right;          /* case 1/2: no left child  */
    else if (root->right == NULL)
        replacement = root->left;           /* case 2: no right child   */
    else
    {
        /* case 3: two children -> inorder successor (leftmost of right subtree).
           The successor node itself is relinked into this position, so its
           route/seat/passenger data moves with it and nothing is copied. */
        parent = root;
        succ = root->right;
        while (succ->left != NULL)
        {
            parent = succ;
            succ = succ->left;
        }
        if (parent != root)
        {
            parent->left = succ->right;
            succ->right = root->right;
        }
        succ->left = root->left;
        replacement = succ;
    }

    freeTrain(root);
    *deleted = 1;
    return replacement;
}

void inorderTrains(TrainNode *root)
{
    if (root == NULL)
        return;
    inorderTrains(root->left);
    printf("%d - %-22.22s %-12.12s -> %-12.12s Seats %2d/%d  WL %d\n",
           root->trainNumber, root->trainName, root->source, root->destination,
           countAvailableSeats(root->seats), TOTAL_SEATS, root->waitingList.size);
    inorderTrains(root->right);
}

int countTrains(TrainNode *root)
{
    if (root == NULL)
        return 0;
    return 1 + countTrains(root->left) + countTrains(root->right);
}

void freeTrain(TrainNode *train)
{
    if (train == NULL)
        return;
    freeRoute(&train->route);
    freeCoaches(&train->coaches);
    train->nextCoach = NULL;
    freePassengerList(&train->passengers);
    freeQueue(&train->waitingList);
    free(train);
}

void freeAllTrains(TrainNode *root)
{
    if (root == NULL)
        return;
    freeAllTrains(root->left);
    freeAllTrains(root->right);
    freeTrain(root);
}
