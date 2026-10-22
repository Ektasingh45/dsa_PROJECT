/*
 * route_dll.c
 * -----------
 * DOUBLY LINKED LIST of stations (the route of one train).
 *
 *   NULL <- Mumbai <-> Surat <-> Vadodara <-> ... <-> Delhi -> NULL
 *           (routeHead)                              (routeTail)
 */
#include "railway.h"

/* Compares two names without caring about capital letters ("agra" == "Agra"). */
int sameName(char first[], char second[])
{
    int i = 0;
    char a, b;

    while (first[i] != '\0' && second[i] != '\0')
    {
        a = first[i];
        b = second[i];
        if (a >= 'A' && a <= 'Z')
            a = a + 32;     /* convert to small letter */
        if (b >= 'A' && b <= 'Z')
            b = b + 32;
        if (a != b)
            return 0;
        i++;
    }
    return first[i] == second[i];   /* both strings must end together */
}

/* Returns the station with this name, or NULL if it is not on the route. */
struct Station *searchStation(struct TrainNode *train, char name[])
{
    struct Station *station = train->routeHead;

    while (station != NULL)
    {
        if (sameName(station->name, name))
            return station;
        station = station->next;
    }
    return NULL;
}

/* Returns the position of a station (0 = first station), or -1 if not found. */
int getStationPosition(struct TrainNode *train, char name[])
{
    struct Station *station = train->routeHead;
    int position = 0;

    while (station != NULL)
    {
        if (sameName(station->name, name))
            return position;
        position++;
        station = station->next;
    }
    return -1;
}

/* Counts the stations on the route. */
int countStations(struct TrainNode *train)
{
    struct Station *station = train->routeHead;
    int count = 0;

    while (station != NULL)
    {
        count++;
        station = station->next;
    }
    return count;
}

/* Adds a new station at the end of the route. */
void addStationAtEnd(struct TrainNode *train, char name[])
{
    struct Station *station = malloc(sizeof(struct Station));
    checkMemory(station);

    strcpy(station->name, name);
    station->next = NULL;
    station->prev = train->routeTail;

    if (train->routeHead == NULL)
        train->routeHead = station;         /* first station of the route */
    else
        train->routeTail->next = station;

    train->routeTail = station;
}

/* Inserts a new station right after the station 'position'. */
void insertStationAfter(struct TrainNode *train, struct Station *position, char name[])
{
    struct Station *station = malloc(sizeof(struct Station));
    checkMemory(station);

    strcpy(station->name, name);
    station->prev = position;
    station->next = position->next;

    if (position->next != NULL)
        position->next->prev = station;
    else
        train->routeTail = station;         /* inserted at the end */

    position->next = station;
}

/* Inserts a new station just before the final destination. */
void addStationBeforeDestination(struct TrainNode *train, char name[])
{
    insertStationAfter(train, train->routeTail->prev, name);
}

/* Unlinks a station from the route and frees it. */
void deleteStation(struct TrainNode *train, struct Station *station)
{
    if (station->prev != NULL)
        station->prev->next = station->next;
    else
        train->routeHead = station->next;   /* deleting the first station */

    if (station->next != NULL)
        station->next->prev = station->prev;
    else
        train->routeTail = station->prev;   /* deleting the last station */

    free(station);
}

/* Prints the route from source to destination using the 'next' links. */
void displayRouteForward(struct TrainNode *train)
{
    struct Station *station = train->routeHead;

    while (station != NULL)
    {
        printf("  %s\n", station->name);
        if (station->next != NULL)
            printf("   |\n   v\n");
        station = station->next;
    }
}

/* Prints the route from destination to source using the 'prev' links. */
void displayRouteBackward(struct TrainNode *train)
{
    struct Station *station = train->routeTail;

    while (station != NULL)
    {
        printf("  %s\n", station->name);
        if (station->prev != NULL)
            printf("   |\n   v\n");
        station = station->prev;
    }
}

/* Prints the route on one line:  A <-> B <-> C */
void displayRouteInLine(struct TrainNode *train)
{
    struct Station *station = train->routeHead;

    while (station != NULL)
    {
        printf("%s", station->name);
        if (station->next != NULL)
            printf(" <-> ");
        station = station->next;
    }
    printf("\n");
}

/* Frees every station of the route. */
void freeRoute(struct TrainNode *train)
{
    struct Station *station = train->routeHead;
    struct Station *nextStation;

    while (station != NULL)
    {
        nextStation = station->next;
        free(station);
        station = nextStation;
    }
    train->routeHead = NULL;
    train->routeTail = NULL;
}
