/*
 * route_dll.c - DOUBLY LINKED LIST of stations (the route of a train).
 * head <-> ... <-> tail : next goes forward, prev goes backward
 */
#include "railway.h"

/* Compares two names ignoring capital letters ("delhi" is the same as "Delhi"). */
int sameName(char a[], char b[]) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z') x = x + 32;   /* make small letter */
        if (y >= 'A' && y <= 'Z') y = y + 32;
        if (x != y) return 0;
        i++;
    }
    return a[i] == b[i];                    /* both must end together */
}

/* Adds a station at the end of the train's route. */
void addStation(Train *t, char name[]) {
    Station *newNode = malloc(sizeof(Station));
    if (!newNode) return;
    strcpy(newNode->name, name);
    newNode->next = NULL;
    newNode->prev = t->tail;
    if (t->head == NULL) t->head = newNode;
    else t->tail->next = newNode;
    t->tail = newNode;
}

/* Prints the route from the first station to the last (uses next). */
void displayForward(Train *t) {
    Station *temp = t->head;
    while (temp != NULL) {
        printf("%s", temp->name);
        if (temp->next != NULL) printf(" -> ");
        temp = temp->next;
    }
    printf("\n");
}

/* Prints the route from the last station to the first (uses prev). */
void displayBackward(Train *t) {
    Station *temp = t->tail;
    while (temp != NULL) {
        printf("%s", temp->name);
        if (temp->prev != NULL) printf(" -> ");
        temp = temp->prev;
    }
    printf("\n");
}

/* Returns the position of a station on the route (0, 1, 2, ...) or -1 if not found. */
int stationIndex(Train *t, char name[]) {
    Station *temp = t->head;
    int i = 0;
    while (temp != NULL) {
        if (sameName(temp->name, name)) return i;
        temp = temp->next;
        i++;
    }
    return -1;
}
