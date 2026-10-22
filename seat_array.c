/*
 * seat_array.c - 2D ARRAY of seats: seats[5][10]
 * row = coach (C1..C5), column = seat (1..10), value 0 = free, 1 = booked
 */
#include "railway.h"

/* Counts the free seats (value 0) of a train. */
int countFreeSeats(Train *t) {
    int count = 0;
    for (int c = 0; c < 5; c++)
        for (int s = 0; s < 10; s++)
            if (t->seats[c][s] == 0) count++;
    return count;
}

/* Prints the seat array, one row per coach (0 = free, 1 = booked). */
void showSeats(Train *t) {
    for (int c = 0; c < 5; c++) {
        printf("C%d: ", c + 1);
        for (int s = 0; s < 10; s++) printf("%d ", t->seats[c][s]);
        printf("\n");
    }
    printf("Free seats: %d / 50\n", countFreeSeats(t));
}

/* Gives p the first free seat starting from the current coach, then moves to the
   next coach, so bookings rotate C1, C2, ..., C5, C1. Returns 0 if the train is full. */
int allotSeat(Train *t, Passenger *p) {
    for (int i = 0; i < 5; i++) {
        int c = t->currentCoach->number - 1;
        for (int s = 0; s < 10; s++) {
            if (t->seats[c][s] == 0) {
                t->seats[c][s] = 1;
                p->coach = c + 1;
                p->seat = s + 1;
                nextCoach(t);
                return 1;
            }
        }
        nextCoach(t);           /* this coach is full, try the next one */
    }
    return 0;
}
