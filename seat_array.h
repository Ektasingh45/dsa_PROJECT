/*
 * seat_array.h
 * ------------
 * ARRAY module - seat management for one train.
 *
 * Every train owns a 2D integer array  seats[MAX_COACHES][SEATS_PER_COACH]
 *   row    -> coach  (C1..C5)
 *   column -> seat   (01..10)
 *   value  -> SEAT_AVAILABLE (0) or SEAT_BOOKED (1)
 *
 * Coach and seat numbers used by the rest of the program are 1-based,
 * this module converts them to 0-based array indices internally.
 */
#ifndef SEAT_ARRAY_H
#define SEAT_ARRAY_H

#define MAX_COACHES      5
#define SEATS_PER_COACH  10
#define TOTAL_SEATS      (MAX_COACHES * SEATS_PER_COACH)

#define SEAT_AVAILABLE   0
#define SEAT_BOOKED      1

void initSeats(int seats[MAX_COACHES][SEATS_PER_COACH]);

void displayCoachSeats(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber);
void displaySeats(int seats[MAX_COACHES][SEATS_PER_COACH]);

/* Returns first free seat number (1..10) in the coach, or 0 if the coach is full. */
int findAvailableSeatInCoach(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber);

/* 1 = success, 0 = seat already in that state, -1 = invalid coach/seat */
int bookSeat(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber);
int releaseSeat(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber);

/* 1 if the seat exists and is free, otherwise 0 */
int isSeatAvailable(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber, int seatNumber);

int countAvailableInCoach(int seats[MAX_COACHES][SEATS_PER_COACH], int coachNumber);
int countAvailableSeats(int seats[MAX_COACHES][SEATS_PER_COACH]);

#endif
