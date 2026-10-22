/*
 * main.c
 * ------
 * Entry point of the Railway Reservation and Management System.
 *
 * Build:  gcc main.c railway.c train_bst.c route_dll.c passenger_sll.c coach_cll.c
 *             seat_array.c waiting_queue.c undo_stack.c -o railway
 */
#include "railway.h"

int main(void)
{
    loadInitialData();
    runRailwaySystem();
    shutdownRailwaySystem();
    return 0;
}
