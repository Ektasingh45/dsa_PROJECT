/*
 * seat_array.c
 * ------------
 * ARRAY of seats:  int seats[5][10]
 *   row    = coach (C1..C5)      -> index coachNumber - 1
 *   column = seat  (01..10)      -> index seatNumber - 1
 *   value  = 0 available, 1 booked
 */
#include "railway.h"

/* Marks every seat as available. */
void initSeats(struct TrainNode *train)
{
    int coach, seat;

    for (coach = 0; coach < MAX_COACHES; coach++)
        for (seat = 0; seat < SEATS_PER_COACH; seat++)
            train->seats[coach][seat] = SEAT_AVAILABLE;
}

/* Prints the seat map of every coach (A = available, X = booked). */
void displaySeats(struct TrainNode *train)
{
    int coach, seat;

    for (coach = 1; coach <= MAX_COACHES; coach++)
    {
        printf("Coach C%d:   (%d/%d available)\n\n", coach,
               countFreeSeatsInCoach(train, coach), SEATS_PER_COACH);

        for (seat = 1; seat <= SEATS_PER_COACH; seat++)
        {
            if (train->seats[coach - 1][seat - 1] == SEAT_BOOKED)
                printf("%02d[X] ", seat);
            else
                printf("%02d[A] ", seat);

            if (seat % 5 == 0)
                printf("\n");
        }
        printf("\n");
    }
    printf("A = Available    X = Booked\n");
    printf("Total available seats: %d / %d\n", countFreeSeats(train), TOTAL_SEATS);
}

/* Counts free seats in one coach. */
int countFreeSeatsInCoach(struct TrainNode *train, int coachNumber)
{
    int seat, count = 0;

    for (seat = 0; seat < SEATS_PER_COACH; seat++)
        if (train->seats[coachNumber - 1][seat] == SEAT_AVAILABLE)
            count++;
    return count;
}

/* Counts free seats in the whole train. */
int countFreeSeats(struct TrainNode *train)
{
    int coach, count = 0;

    for (coach = 1; coach <= MAX_COACHES; coach++)
        count += countFreeSeatsInCoach(train, coach);
    return count;
}

/* Returns the first free seat number (1..10) in a coach, or 0 if the coach is full. */
int findFreeSeat(struct TrainNode *train, int coachNumber)
{
    int seat;

    for (seat = 1; seat <= SEATS_PER_COACH; seat++)
        if (train->seats[coachNumber - 1][seat - 1] == SEAT_AVAILABLE)
            return seat;
    return 0;
}

/* Returns 1 if this seat exists and is free. */
int isSeatFree(struct TrainNode *train, int coachNumber, int seatNumber)
{
    if (coachNumber < 1 || coachNumber > MAX_COACHES)
        return 0;
    if (seatNumber < 1 || seatNumber > SEATS_PER_COACH)
        return 0;
    return train->seats[coachNumber - 1][seatNumber - 1] == SEAT_AVAILABLE;
}

/* Marks a seat as booked. */
void bookSeat(struct TrainNode *train, int coachNumber, int seatNumber)
{
    train->seats[coachNumber - 1][seatNumber - 1] = SEAT_BOOKED;
}

/* Marks a seat as available again. */
void releaseSeat(struct TrainNode *train, int coachNumber, int seatNumber)
{
    train->seats[coachNumber - 1][seatNumber - 1] = SEAT_AVAILABLE;
}
