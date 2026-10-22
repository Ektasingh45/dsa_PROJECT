/*
 * seat_array.c
 * ------------
 * Implementation of the seat ARRAY operations.
 * Accessing seats[c][s] is O(1) - that is the reason an array is used.
 */
#include <stdio.h>
#include "seat_array.h"

static int isValidPosition(int coachNumber, int seatNumber)
{
    return coachNumber >= 1 && coachNumber <= MAX_COACHES &&
           seatNumber >= 1 && seatNumber <= SEATS_PER_COACH;
}

void initSeats(int seats[MAX_COACHES][SEATS_PER_COACH])
{
    int c, s;
    for (c = 0; c < MAX_COACHES; c++)
        for (s = 0; s < SEATS_PER_COACH; s++)
            seats[c][s] = SEAT_AVAILABLE;
}

void displayCoachSeats(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber)
{
    int s;
    if (coachNumber < 1 || coachNumber > MAX_COACHES)
    {
        printf("Invalid coach number.\n");
        return;
    }

    printf("Coach C%d:   (%d/%d available)\n\n", coachNumber,
           countAvailableInCoach(seats, coachNumber), SEATS_PER_COACH);
    for (s = 0; s < SEATS_PER_COACH; s++)
    {
        printf("%02d[%c] ", s + 1,
               seats[coachNumber - 1][s] == SEAT_BOOKED ? 'X' : 'A');
        if ((s + 1) % 5 == 0)
            printf("\n");
    }
    printf("\n");
}

void displaySeats(int seats[MAX_COACHES][SEATS_PER_COACH])
{
    int c;
    for (c = 1; c <= MAX_COACHES; c++)
        displayCoachSeats(seats, c);

    printf("A = Available    X = Booked\n");
    printf("Total available seats: %d / %d\n", countAvailableSeats(seats), TOTAL_SEATS);
}

int findAvailableSeatInCoach(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber)
{
    int s;
    if (coachNumber < 1 || coachNumber > MAX_COACHES)
        return 0;

    for (s = 0; s < SEATS_PER_COACH; s++)
        if (seats[coachNumber - 1][s] == SEAT_AVAILABLE)
            return s + 1;
    return 0;
}

int bookSeat(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber)
{
    if (!isValidPosition(coachNumber, seatNumber))
        return -1;
    if (seats[coachNumber - 1][seatNumber - 1] == SEAT_BOOKED)
        return 0;

    seats[coachNumber - 1][seatNumber - 1] = SEAT_BOOKED;
    return 1;
}

int releaseSeat(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber)
{
    if (!isValidPosition(coachNumber, seatNumber))
        return -1;
    if (seats[coachNumber - 1][seatNumber - 1] == SEAT_AVAILABLE)
        return 0;

    seats[coachNumber - 1][seatNumber - 1] = SEAT_AVAILABLE;
    return 1;
}

int isSeatAvailable(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber)
{
    if (!isValidPosition(coachNumber, seatNumber))
        return 0;
    return seats[coachNumber - 1][seatNumber - 1] == SEAT_AVAILABLE;
}

int countAvailableInCoach(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber)
{
    int s, count = 0;
    if (coachNumber < 1 || coachNumber > MAX_COACHES)
        return 0;

    for (s = 0; s < SEATS_PER_COACH; s++)
        if (seats[coachNumber - 1][s] == SEAT_AVAILABLE)
            count++;
    return count;
}

int countAvailableSeats(int seats[MAX_COACHES][SEATS_PER_COACH])
{
    int c, count = 0;
    for (c = 1; c <= MAX_COACHES; c++)
        count += countAvailableInCoach(seats, c);
    return count;
}
