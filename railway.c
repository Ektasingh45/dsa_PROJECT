/*
 * railway.c - admin and passenger operations.
 * This file makes the data structures work together
 * (booking, cancellation, waiting list promotion and undo).
 */
#include "railway.h"

Train *root = NULL;             /* root of the BST */
int nextPnr = 100001;           /* next PNR to give out */

/* ======================= INPUT ======================= */
/* Reads a number. Returns -1 if the user typed something that is not a number. */
int readNumber() {
    int x;
    if (scanf("%d", &x) == 1) return x;
    if (feof(stdin)) exit(0);   /* no more input */
    scanf("%*s");               /* skip the wrong word */
    return -1;
}

/* Reads a line of text, spaces allowed (e.g. "New Delhi"). */
void readText(char text[]) {
    if (scanf(" %49[^\n]", text) != 1) exit(0);
    int n = strlen(text);
    while (n > 0 && text[n - 1] == ' ') {   /* remove spaces at the end */
        n--;
        text[n] = '\0';
    }
}

/* ======================= HELPERS ======================= */
/* Asks for a train number and finds the train in the BST. */
Train *askTrain() {
    printf("Enter train number: ");
    Train *t = searchTrain(root, readNumber());
    if (t == NULL) printf("Train not found.\n");
    return t;
}

/* Prints a ticket. */
void printTicket(Train *t, Passenger *p) {
    printf("\n------------ TICKET ------------\n");
    printf("PNR Number : %d\n", p->pnr);
    printf("Name       : %s\n", p->name);
    printf("Age        : %d\n", p->age);
    printf("Train No.  : %d\n", t->number);
    printf("Train Name : %s\n", t->name);
    printf("From       : %s\n", p->from);
    printf("To         : %s\n", p->to);
    printf("Fare       : Rs. %d\n", p->fare);
    if (p->coach == 0) {
        printf("Status     : WAITING LIST\n");
    } else {
        printf("Status     : CONFIRMED\n");
        printf("Coach      : C%d\n", p->coach);
        printf("Seat       : %d\n", p->seat);
    }
    printf("--------------------------------\n");
}

/* Gives a free seat to the first passenger in the waiting queue.
   Returns the PNR of the promoted passenger, or 0 if nobody was promoted. */
int promoteWaiting(Train *t) {
    Passenger *p = t->front;
    if (p == NULL || !allotSeat(t, p)) return 0;
    dequeue(t);
    addPassenger(t, p);
    printf("%s (PNR %d) moved from waiting list to seat C%d-%d.\n", p->name, p->pnr, p->coach, p->seat);
    return p->pnr;
}

/* Reads passenger details. Asks From/To again until both are on the route in the
   right direction. Returns NULL if the passenger types 0 to cancel. */
Passenger *readPassenger(Train *t) {
    Passenger *p = malloc(sizeof(Passenger));
    if (!p) return NULL;
    printf("Name: ");  readText(p->name);
    printf("Age: ");   p->age = readNumber();
    printf("Stations: ");
    displayForward(t);
    while (1) {
        printf("From (0 to cancel): ");
        readText(p->from);
        if (strcmp(p->from, "0") == 0) { free(p); return NULL; }
        printf("To: ");
        readText(p->to);
        int a = stationIndex(t, p->from);
        int b = stationIndex(t, p->to);
        if (a != -1 && b != -1 && a < b) {
            p->fare = 250 + 240 * (b - a);
            return p;
        }
        if (a == -1) printf("'%s' is not on this route.\n", p->from);
        else if (b == -1) printf("'%s' is not on this route.\n", p->to);
        else printf("Wrong direction: this train goes %s -> %s.\n", t->head->name, t->tail->name);
    }
}

/* ======================= ADMIN ======================= */
/* Admin: shows all trains with a heading (BST inorder = sorted by number). */
void showAllTrains() {
    printf("\n------------- ALL TRAINS (sorted by number) -------------\n");
    displayTrains(root);
    printf("---------------------------------------------------------\n");
}

/* Admin: creates a new train and inserts it into the BST. */
void addTrain() {
    char name[50];
    printf("Train number: ");
    int number = readNumber();
    if (number <= 0 || searchTrain(root, number) != NULL) { printf("Invalid or duplicate train number.\n"); return; }
    printf("Train name: ");
    readText(name);
    root = insertTrain(root, createTrain(number, name));
    printf("Train added. Now add its stations in order (option 3).\n");
}

/* Admin: adds a station at the end of a train's route. */
void addStationMenu() {
    char name[50];
    Train *t = askTrain();
    if (t == NULL) return;
    printf("Station name: ");
    readText(name);
    if (stationIndex(t, name) != -1) { printf("Station is already on the route.\n"); return; }
    addStation(t, name);
    printf("Route: ");
    displayForward(t);
}

/* ======================= PASSENGER ======================= */
/* Books a ticket on train t. If the train is full, the passenger joins the waiting queue. */
void bookOnTrain(Train *t) {
    Passenger *p = readPassenger(t);
    if (p == NULL) return;
    p->pnr = nextPnr;
    nextPnr++;
    if (allotSeat(t, p)) {
        addPassenger(t, p);
        push(1, t->number, p, 0);
        printf("Booking confirmed!\n");
    } else {
        p->coach = 0;
        p->seat = 0;
        enqueue(t, p);
        printf("Train is full. Added to the waiting list.\n");
    }
    printTicket(t, p);
}

/* Asks for a train and books a ticket on it. */
void bookTicket() {
    Train *t = askTrain();
    if (t != NULL) bookOnTrain(t);
}

/* After viewing a train: book a ticket on it, or go back.
   Returns 1 if the passenger wants to go back to the main menu. */
int afterViewMenu(Train *t) {
    printf("\n1. Book a ticket on this train\n2. Back to passenger menu\n3. Back to main menu\nChoice: ");
    int choice = readNumber();
    if (choice == 1) bookOnTrain(t);
    return choice == 3;
}

/* Shows a train's route forward and backward, then offers to book. */
int viewRoute() {
    Train *t = askTrain();
    if (t == NULL) return 0;
    printf("Forward : ");
    displayForward(t);
    printf("Backward: ");
    displayBackward(t);
    return afterViewMenu(t);
}

/* Shows the seat array of a train, then offers to book. */
int checkSeats() {
    Train *t = askTrain();
    if (t == NULL) return 0;
    showSeats(t);
    return afterViewMenu(t);
}

/* Cancels a confirmed ticket, frees the seat and gives it to the first waiting passenger. */
void cancelTicket() {
    printf("Enter PNR: ");
    int pnr = readNumber();
    Train *t = findTrainOfPnr(root, pnr);
    Passenger *p = NULL;
    if (t != NULL) p = deletePassenger(t, pnr);
    if (p == NULL) { printf("No confirmed ticket with this PNR.\n"); return; }
    t->seats[p->coach - 1][p->seat - 1] = 0;
    printf("Ticket cancelled. Seat C%d-%d is free.\n", p->coach, p->seat);
    int promoted = promoteWaiting(t);
    push(2, t->number, p, promoted);
    free(p);
}

/* Shows a ticket (confirmed or waiting) using its PNR. */
void viewTicket() {
    printf("Enter PNR: ");
    int pnr = readNumber();
    Train *t = findTrainOfPnr(root, pnr);
    if (t == NULL) { printf("PNR not found.\n"); return; }
    Passenger *p = searchPnr(t->passengers, pnr);
    if (p == NULL) p = searchPnr(t->front, pnr);
    printTicket(t, p);
}

/* ======================= UNDO ======================= */
/* Undo a booking: remove the passenger and free the seat. */
void undoBooking(Train *t, Undo *u) {
    Passenger *p = deletePassenger(t, u->data.pnr);
    if (p == NULL) { printf("Ticket %d is no longer booked. Nothing to undo.\n", u->data.pnr); return; }
    t->seats[p->coach - 1][p->seat - 1] = 0;
    printf("Undo: booking %d removed, seat C%d-%d is free.\n", p->pnr, p->coach, p->seat);
    free(p);
    promoteWaiting(t);
}

/* Undo a cancellation: send the promoted passenger back to the queue, then restore the ticket. */
void undoCancel(Train *t, Undo *u) {
    if (u->promotedPnr != 0) sendBackToQueue(t, u->promotedPnr);
    Passenger *p = malloc(sizeof(Passenger));
    if (!p) return;
    *p = u->data;                           /* copy the saved ticket back */
    if (t->seats[p->coach - 1][p->seat - 1] == 0)
        t->seats[p->coach - 1][p->seat - 1] = 1;    /* same seat as before */
    else if (!allotSeat(t, p)) {
        printf("No free seat to restore the ticket.\n");
        free(p);
        return;
    }
    addPassenger(t, p);
    printf("Undo: ticket %d of %s restored in seat C%d-%d.\n", p->pnr, p->name, p->coach, p->seat);
}

/* Undoes the last booking or cancellation (top of the stack). */
void undoLast() {
    Undo *u = pop();
    if (u == NULL) { printf("Nothing to undo.\n"); return; }
    Train *t = searchTrain(root, u->trainNo);
    if (u->type == 1) undoBooking(t, u);
    else undoCancel(t, u);
    free(u);
}
