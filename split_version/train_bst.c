/*
 * train_bst.c - BINARY SEARCH TREE of trains (key = train number).
 * Smaller numbers go left, bigger numbers go right.
 */
#include "railway.h"

/* Creates a train with an empty route, free seats, 5 coaches and empty lists. */
Train *createTrain(int number, char name[]) {
    Train *t = malloc(sizeof(Train));
    if (!t) exit(1);
    t->number = number;
    strcpy(t->name, name);
    for (int c = 0; c < 5; c++)
        for (int s = 0; s < 10; s++)
            t->seats[c][s] = 0;
    t->currentCoach = createCoaches();
    t->head = t->tail = NULL;
    t->passengers = t->front = t->rear = NULL;
    t->left = t->right = NULL;
    return t;
}

/* Inserts a train into the BST: smaller numbers go left, bigger go right. */
Train *insertTrain(Train *node, Train *t) {
    if (node == NULL) return t;
    if (t->number < node->number) node->left = insertTrain(node->left, t);
    else node->right = insertTrain(node->right, t);
    return node;
}

/* Searches the BST for a train number. Returns NULL if not found. */
Train *searchTrain(Train *node, int number) {
    if (node == NULL || node->number == number) return node;
    if (number < node->number) return searchTrain(node->left, number);
    return searchTrain(node->right, number);
}

/* Inorder traversal: prints all trains sorted by train number. */
void displayTrains(Train *node) {
    if (node == NULL) return;
    displayTrains(node->left);
    printf("%d  %-20s  ", node->number, node->name);
    if (node->head != NULL) printf("%s -> %s", node->head->name, node->tail->name);
    printf("  (free seats: %d)\n", countFreeSeats(node));
    displayTrains(node->right);
}

/* Visits every train in the BST and returns the one that has this PNR. */
Train *findTrainOfPnr(Train *node, int pnr) {
    if (node == NULL) return NULL;
    if (searchPnr(node->passengers, pnr) != NULL || searchPnr(node->front, pnr) != NULL)
        return node;
    Train *found = findTrainOfPnr(node->left, pnr);
    if (found == NULL) found = findTrainOfPnr(node->right, pnr);
    return found;
}
