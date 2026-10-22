/*
 * train_bst.c
 * -----------
 * BINARY SEARCH TREE of trains. The key is the train number.
 * Smaller numbers go to the left, bigger numbers go to the right.
 */
#include "railway.h"

/* Creates a new train and sets up ALL of its own data structures. */
struct TrainNode *createTrain(int trainNumber, char name[], char source[], char destination[])
{
    struct TrainNode *train = malloc(sizeof(struct TrainNode));
    checkMemory(train);

    train->trainNumber = trainNumber;
    strcpy(train->trainName, name);
    strcpy(train->source, source);
    strcpy(train->destination, destination);

    train->routeHead = NULL;                /* 1. route (doubly linked list) */
    train->routeTail = NULL;
    addStationAtEnd(train, source);
    addStationAtEnd(train, destination);

    createCoaches(train);                   /* 2. coaches (circular linked list) */
    initSeats(train);                       /* 3. seats (array) */

    train->passengerHead = NULL;            /* 4. empty passenger list (singly linked list) */
    train->waitFront = NULL;                /* 5. empty waiting list (queue) */
    train->waitRear = NULL;

    train->left = NULL;
    train->right = NULL;
    return train;
}

/* Inserts a train into the BST and returns the (new) root. */
struct TrainNode *insertTrain(struct TrainNode *root, struct TrainNode *newTrain)
{
    if (root == NULL)
        return newTrain;

    if (newTrain->trainNumber < root->trainNumber)
        root->left = insertTrain(root->left, newTrain);
    else
        root->right = insertTrain(root->right, newTrain);
    return root;
}

/* Searches the BST for a train number. Returns NULL if not found. */
struct TrainNode *searchTrain(struct TrainNode *root, int trainNumber)
{
    if (root == NULL || root->trainNumber == trainNumber)
        return root;

    if (trainNumber < root->trainNumber)
        return searchTrain(root->left, trainNumber);
    else
        return searchTrain(root->right, trainNumber);
}

/* Returns the train with the smallest number (the leftmost node). */
struct TrainNode *findMinimum(struct TrainNode *root)
{
    while (root->left != NULL)
        root = root->left;
    return root;
}

/*
 * Removes the leftmost node of a subtree.
 * Only the node itself is freed - its data was already copied somewhere else.
 */
struct TrainNode *removeMinimum(struct TrainNode *root)
{
    struct TrainNode *rightChild;

    if (root->left == NULL)
    {
        rightChild = root->right;
        free(root);
        return rightChild;
    }
    root->left = removeMinimum(root->left);
    return root;
}

/* Copies all train data from 'source' into 'destination' but keeps destination's children. */
void copyTrainData(struct TrainNode *destination, struct TrainNode *source)
{
    struct TrainNode *leftChild = destination->left;
    struct TrainNode *rightChild = destination->right;

    *destination = *source;
    destination->left = leftChild;
    destination->right = rightChild;
}

/* Deletes a train from the BST, frees all its memory and returns the (new) root. */
struct TrainNode *deleteTrain(struct TrainNode *root, int trainNumber)
{
    struct TrainNode *child, *successor;

    if (root == NULL)
        return NULL;

    if (trainNumber < root->trainNumber)
        root->left = deleteTrain(root->left, trainNumber);
    else if (trainNumber > root->trainNumber)
        root->right = deleteTrain(root->right, trainNumber);
    else
    {
        /* Case 1 and 2: node has no child or only one child */
        if (root->left == NULL)
        {
            child = root->right;
            freeTrain(root);
            return child;
        }
        if (root->right == NULL)
        {
            child = root->left;
            freeTrain(root);
            return child;
        }

        /* Case 3: two children -> replace with the inorder successor */
        successor = findMinimum(root->right);
        freeTrainData(root);                      /* free the deleted train's lists */
        copyTrainData(root, successor);           /* successor's train moves here   */
        root->right = removeMinimum(root->right); /* remove the old successor node  */
    }
    return root;
}

/* INORDER traversal (left, root, right) prints the trains sorted by number. */
void displayAllTrains(struct TrainNode *root)
{
    if (root == NULL)
        return;

    displayAllTrains(root->left);
    printf("%d - %-22.22s %-12.12s -> %-12.12s Seats %2d/%d  WL %d\n",
           root->trainNumber, root->trainName, root->source, root->destination,
           countFreeSeats(root), TOTAL_SEATS, countWaiting(root));
    displayAllTrains(root->right);
}

/* Counts the trains in the BST. */
int countTrains(struct TrainNode *root)
{
    if (root == NULL)
        return 0;
    return 1 + countTrains(root->left) + countTrains(root->right);
}

/* Frees everything a train owns: route, coaches, passengers and waiting list. */
void freeTrainData(struct TrainNode *train)
{
    freeRoute(train);
    freeCoaches(train);
    freePassengers(train);
    freeWaitingList(train);
}

/* Frees a train's data and the train node itself. */
void freeTrain(struct TrainNode *train)
{
    freeTrainData(train);
    free(train);
}

/* Frees the whole BST (postorder: children first, then the node). */
void freeAllTrains(struct TrainNode *root)
{
    if (root == NULL)
        return;

    freeAllTrains(root->left);
    freeAllTrains(root->right);
    freeTrain(root);
}
