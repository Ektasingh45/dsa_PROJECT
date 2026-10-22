/*
 * route_dll.c
 * -----------
 * Implementation of the route DOUBLY LINKED LIST.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "route_dll.h"

static char toLowerChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return (char)(c - 'A' + 'a');
    return c;
}

int namesEqual(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0')
    {
        if (toLowerChar(*a) != toLowerChar(*b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static Station *createStation(const char *name)
{
    Station *s = (Station *)malloc(sizeof(Station));
    if (s == NULL)
    {
        printf("Memory allocation failed for station.\n");
        return NULL;
    }
    strncpy(s->name, name, NAME_LEN - 1);
    s->name[NAME_LEN - 1] = '\0';
    s->prev = NULL;
    s->next = NULL;
    return s;
}

/* Links an already allocated station right after 'pos'. */
static void linkAfter(Route *route, Station *pos, Station *s)
{
    s->prev = pos;
    s->next = pos->next;
    if (pos->next != NULL)
        pos->next->prev = s;
    else
        route->tail = s;
    pos->next = s;
    route->count++;
}

void initRoute(Route *route)
{
    route->head = NULL;
    route->tail = NULL;
    route->count = 0;
}

Station *searchStation(const Route *route, const char *name)
{
    Station *cur = route->head;
    while (cur != NULL)
    {
        if (namesEqual(cur->name, name))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

int stationIndex(const Route *route, const char *name)
{
    Station *cur = route->head;
    int index = 0;
    while (cur != NULL)
    {
        if (namesEqual(cur->name, name))
            return index;
        index++;
        cur = cur->next;
    }
    return -1;
}

int addStationAtEnd(Route *route, const char *name)
{
    Station *s;
    if (searchStation(route, name) != NULL)
        return ROUTE_DUPLICATE;

    s = createStation(name);
    if (s == NULL)
        return ROUTE_NOMEM;

    s->prev = route->tail;
    if (route->tail != NULL)
        route->tail->next = s;
    else
        route->head = s;
    route->tail = s;
    route->count++;
    return ROUTE_OK;
}

int addStationAfter(Route *route, const char *afterName, const char *name)
{
    Station *pos, *s;
    if (searchStation(route, name) != NULL)
        return ROUTE_DUPLICATE;

    pos = searchStation(route, afterName);
    if (pos == NULL)
        return ROUTE_NOT_FOUND;

    s = createStation(name);
    if (s == NULL)
        return ROUTE_NOMEM;

    linkAfter(route, pos, s);
    return ROUTE_OK;
}

int addStationBeforeLast(Route *route, const char *name)
{
    Station *s;
    if (route->count < 2)
        return addStationAtEnd(route, name);
    if (searchStation(route, name) != NULL)
        return ROUTE_DUPLICATE;

    s = createStation(name);
    if (s == NULL)
        return ROUTE_NOMEM;

    linkAfter(route, route->tail->prev, s);
    return ROUTE_OK;
}

int deleteStation(Route *route, const char *name)
{
    Station *s = searchStation(route, name);
    if (s == NULL)
        return ROUTE_NOT_FOUND;

    if (s->prev != NULL)
        s->prev->next = s->next;
    else
        route->head = s->next;

    if (s->next != NULL)
        s->next->prev = s->prev;
    else
        route->tail = s->prev;

    free(s);
    route->count--;
    return ROUTE_OK;
}

void displayRouteForward(const Route *route)
{
    Station *cur = route->head;
    if (cur == NULL)
    {
        printf("Route is empty.\n");
        return;
    }
    while (cur != NULL)
    {
        printf("  %s\n", cur->name);
        if (cur->next != NULL)
            printf("   |\n   v\n");
        cur = cur->next;
    }
}

void displayRouteReverse(const Route *route)
{
    Station *cur = route->tail;
    if (cur == NULL)
    {
        printf("Route is empty.\n");
        return;
    }
    while (cur != NULL)
    {
        printf("  %s\n", cur->name);
        if (cur->prev != NULL)
            printf("   |\n   v\n");
        cur = cur->prev;
    }
}

void displayRouteInline(const Route *route)
{
    Station *cur = route->head;
    if (cur == NULL)
    {
        printf("Route is empty.\n");
        return;
    }
    while (cur != NULL)
    {
        printf("%s", cur->name);
        if (cur->next != NULL)
            printf(" <-> ");
        cur = cur->next;
    }
    printf("\n");
}

void freeRoute(Route *route)
{
    Station *cur = route->head, *next;
    while (cur != NULL)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
    initRoute(route);
}
