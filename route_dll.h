/*
 * route_dll.h
 * -----------
 * DOUBLY LINKED LIST module - route of one train.
 *
 *     Mumbai <-> Surat <-> Vadodara <-> Ahmedabad <-> Kota <-> Delhi
 *
 * The prev/next links allow the route to be printed forward (onward
 * journey) and backward (return journey), and allow a station to be
 * inserted/removed in the middle in O(1) once it has been located.
 */
#ifndef ROUTE_DLL_H
#define ROUTE_DLL_H

#ifndef NAME_LEN
#define NAME_LEN 100
#endif

#define ROUTE_OK          1
#define ROUTE_DUPLICATE   0
#define ROUTE_NOMEM      -1
#define ROUTE_NOT_FOUND  -2

typedef struct Station
{
    char name[NAME_LEN];
    struct Station *prev;
    struct Station *next;
} Station;

typedef struct Route
{
    Station *head;   /* source station      */
    Station *tail;   /* destination station */
    int count;
} Route;

void initRoute(Route *route);

/* Case-insensitive string comparison used for station names. 1 = equal. */
int namesEqual(const char *a, const char *b);

int addStationAtEnd(Route *route, const char *name);
int addStationAfter(Route *route, const char *afterName, const char *name);
int addStationBeforeLast(Route *route, const char *name);
int deleteStation(Route *route, const char *name);

Station *searchStation(const Route *route, const char *name);

/* 0-based position of the station in the route, -1 if absent. */
int stationIndex(const Route *route, const char *name);

void displayRouteForward(const Route *route);
void displayRouteReverse(const Route *route);
void displayRouteInline(const Route *route);

void freeRoute(Route *route);

#endif
