/*
 * main.c - Railway Reservation System (start-up and menus).
 *
 * Data structures (one file each):
 *   train_bst.c      Binary Search Tree   -> trains
 *   route_dll.c      Doubly Linked List   -> route of each train
 *   seat_array.c     2D Array             -> seats of each train
 *   passenger_sll.c  Singly Linked List   -> confirmed passengers
 *   waiting_queue.c  Queue                -> waiting list
 *   undo_stack.c     Stack                -> undo
 *   coach_cll.c      Circular Linked List -> coaches
 *   railway.c        booking, cancellation, undo, admin work
 *
 * Compile and run:  run.bat
 * or:  gcc main.c railway.c train_bst.c route_dll.c seat_array.c passenger_sll.c
 *          waiting_queue.c undo_stack.c coach_cll.c -o railway
 */
#include "railway.h"

/* Books all 50 seats of a train (first to last station) so the waiting list can be demonstrated. */
void fillTrain(Train *t) {
    for (int i = 1; i <= 50; i++) {
        Passenger *p = malloc(sizeof(Passenger));
        if (!p) return;
        sprintf(p->name, "Passenger %d", i);
        p->age = 30;
        strcpy(p->from, t->head->name);
        strcpy(p->to, t->tail->name);
        p->fare = 250 + 240 * stationIndex(t, t->tail->name);
        p->pnr = nextPnr;
        nextPnr++;
        allotSeat(t, p);
        addPassenger(t, p);
    }
}

/* Loads 4 trains with their routes at the start of the program.
   Train 12627 is fully booked so the waiting list can be shown in the demo. */
void loadTrains() {
    char *route1[] = {"Mumbai", "Surat", "Vadodara", "Ahmedabad", "Kota", "Delhi"};
    char *route2[] = {"Delhi", "Kota", "Ahmedabad", "Vadodara", "Surat", "Mumbai"};
    char *route3[] = {"Delhi", "Mathura", "Kota", "Ratlam", "Vadodara", "Surat", "Mumbai"};
    char *route4[] = {"New Delhi", "Agra", "Gwalior", "Bhopal", "Nagpur", "Bangalore"};
    Train *t1 = createTrain(12951, "Mumbai Rajdhani");
    Train *t2 = createTrain(12952, "Delhi Rajdhani");
    Train *t3 = createTrain(12431, "Rajdhani Express");
    Train *t4 = createTrain(12627, "Karnataka Express");
    for (int i = 0; i < 6; i++) addStation(t1, route1[i]);
    for (int i = 0; i < 6; i++) addStation(t2, route2[i]);
    for (int i = 0; i < 7; i++) addStation(t3, route3[i]);
    for (int i = 0; i < 6; i++) addStation(t4, route4[i]);
    fillTrain(t4);
    root = insertTrain(root, t1);
    root = insertTrain(root, t2);
    root = insertTrain(root, t3);
    root = insertTrain(root, t4);
    printf("All 4 trains loaded successfully!\n");
    printf("(Train 12627 is FULL to demo the waiting list. Its PNRs are 100001 to 100050.)\n");
}

/* Admin menu (username admin, password 1234). */
void adminMenu() {
    char username[50];
    printf("Username: ");
    readText(username);
    printf("Password: ");
    int password = readNumber();
    if (strcmp(username, "admin") != 0 || password != 1234) {
        printf("Wrong username or password.\n");
        return;
    }
    printf("Login successful. Welcome, Admin!\n");
    int choice = 0;
    while (choice != 4) {
        printf("\n--- ADMIN ---\n1. Add train\n2. Display all trains\n3. Add station to route\n4. Logout\nChoice: ");
        choice = readNumber();
        if (choice == 1) addTrain();
        else if (choice == 2) showAllTrains();
        else if (choice == 3) addStationMenu();
        else if (choice != 4) printf("Invalid choice.\n");
    }
}

/* Passenger menu. */
void passengerMenu() {
    int choice = 0;
    while (choice != 7) {
        printf("\n--- PASSENGER ---\n1. View route\n2. Check seats\n3. Book ticket\n4. Cancel ticket\n");
        printf("5. View ticket\n6. Undo last operation\n7. Logout\nChoice: ");
        choice = readNumber();
        if (choice == 1) {
            if (viewRoute() == 1) return;       /* passenger chose "Back to main menu" */
        }
        else if (choice == 2) {
            if (checkSeats() == 1) return;
        }
        else if (choice == 3) bookTicket();
        else if (choice == 4) cancelTicket();
        else if (choice == 5) viewTicket();
        else if (choice == 6) undoLast();
        else if (choice != 7) printf("Invalid choice.\n");
    }
}

/* Main menu: loads the trains, then lets the user choose Admin or Passenger. */
int main() {
    loadTrains();
    int choice = 0;
    while (choice != 3) {
        printf("\n=== RAILWAY RESERVATION SYSTEM ===\n1. Admin\n2. Passenger\n3. Exit\nChoice: ");
        choice = readNumber();
        if (choice == 1) adminMenu();
        else if (choice == 2) passengerMenu();
        else if (choice != 3) printf("Invalid choice.\n");
    }
    printf("Goodbye!\n");
    return 0;
}
