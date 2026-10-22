/*
 * Railway Reservation System - simple DSA project in one file.
 * BST = trains, Doubly Linked List = route, 2D Array = seats, Singly Linked List =
 * passengers, Queue = waiting list, Stack = undo, Circular Linked List = coaches.
 * Compile: gcc main.c -o railway
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================= STRUCTURES ======================= */
typedef struct Station {        /* one station of a route (doubly linked list) */
    char name[50];
    struct Station *prev, *next;
} Station;

typedef struct Coach {          /* one coach (circular linked list) */
    int number;
    struct Coach *next;
} Coach;

typedef struct Passenger {      /* one ticket (used in the passenger list AND the waiting queue) */
    int pnr, age, fare;
    char name[50], from[50], to[50];
    int coach, seat;            /* coach 0 = no seat yet (waiting) */
    struct Passenger *next;
} Passenger;

typedef struct Train {          /* one train = one node of the BST */
    int number;
    char name[50];
    Station *head, *tail;       /* route: doubly linked list */
    int seats[5][10];           /* seats: 2D array, 0 = free, 1 = booked */
    Coach *currentCoach;        /* coaches: circular linked list */
    Passenger *passengers;      /* confirmed passengers: singly linked list */
    Passenger *front, *rear;    /* waiting list: queue */
    struct Train *left, *right;
} Train;

typedef struct Undo {           /* one saved operation = one node of the stack */
    int type;                   /* 1 = booking, 2 = cancellation */
    int trainNo;
    Passenger data;             /* copy of the ticket */
    int promotedPnr;            /* waiting passenger who got the freed seat (0 = none) */
    struct Undo *next;
} Undo;

Train *root = NULL;             /* root of the BST */
Undo *top = NULL;               /* top of the undo stack */
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

/* Compares two names ignoring capital letters ("delhi" is the same as "Delhi"). */
int sameName(char a[], char b[]) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z') x = x + 32;   /* make small letter */
        if (y >= 'A' && y <= 'Z') y = y + 32;
        if (x != y) return 0;
        i++;
    }
    return a[i] == b[i];                    /* both must end together */
}

/* ======================= CIRCULAR LINKED LIST (coaches) ======================= */
/* Creates 5 coaches in a circle: C1 -> C2 -> C3 -> C4 -> C5 -> back to C1. */
Coach *createCoaches() {
    Coach *head = NULL, *last = NULL;
    for (int i = 1; i <= 5; i++) {
        Coach *newNode = malloc(sizeof(Coach));
        if (!newNode) exit(1);
        newNode->number = i;
        if (head == NULL) head = newNode;
        else last->next = newNode;
        last = newNode;
    }
    last->next = head;          /* last coach points back to the first */
    return head;
}

/* Moves the train to its next coach. After C5 it comes back to C1. */
void nextCoach(Train *t) {
    t->currentCoach = t->currentCoach->next;
}

/* ======================= ARRAY (seats) ======================= */
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

/* ======================= DOUBLY LINKED LIST (route) ======================= */
/* Adds a station at the end of the train's route. */
void addStation(Train *t, char name[]) {
    Station *newNode = malloc(sizeof(Station));
    if (!newNode) return;
    strcpy(newNode->name, name);
    newNode->next = NULL;
    newNode->prev = t->tail;
    if (t->head == NULL) t->head = newNode;
    else t->tail->next = newNode;
    t->tail = newNode;
}

/* Prints the route from the first station to the last (uses next). */
void displayForward(Train *t) {
    Station *temp = t->head;
    while (temp != NULL) {
        printf("%s", temp->name);
        if (temp->next != NULL) printf(" -> ");
        temp = temp->next;
    }
    printf("\n");
}

/* Prints the route from the last station to the first (uses prev). */
void displayBackward(Train *t) {
    Station *temp = t->tail;
    while (temp != NULL) {
        printf("%s", temp->name);
        if (temp->prev != NULL) printf(" -> ");
        temp = temp->prev;
    }
    printf("\n");
}

/* Returns the position of a station on the route (0, 1, 2, ...) or -1 if not found. */
int stationIndex(Train *t, char name[]) {
    Station *temp = t->head;
    int i = 0;
    while (temp != NULL) {
        if (sameName(temp->name, name)) return i;
        temp = temp->next;
        i++;
    }
    return -1;
}

/* ======================= SINGLY LINKED LIST (passengers) ======================= */
/* Adds a passenger at the end of the confirmed list. */
void addPassenger(Train *t, Passenger *p) {
    p->next = NULL;
    if (t->passengers == NULL) { t->passengers = p; return; }
    Passenger *temp = t->passengers;
    while (temp->next != NULL) temp = temp->next;
    temp->next = p;
}

/* Searches a list (confirmed list or waiting queue) for a PNR. */
Passenger *searchPnr(Passenger *head, int pnr) {
    Passenger *temp = head;
    while (temp != NULL) {
        if (temp->pnr == pnr) return temp;
        temp = temp->next;
    }
    return NULL;
}

/* Removes the passenger with this PNR from the confirmed list and returns it. */
Passenger *deletePassenger(Train *t, int pnr) {
    Passenger *temp = t->passengers, *prev = NULL;
    while (temp != NULL && temp->pnr != pnr) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return NULL;          /* not found */
    if (prev == NULL) t->passengers = temp->next;
    else prev->next = temp->next;
    return temp;
}

/* ======================= QUEUE (waiting list) ======================= */
/* Adds a passenger at the rear of the waiting queue. */
void enqueue(Train *t, Passenger *p) {
    p->next = NULL;
    if (t->rear == NULL) t->front = p;
    else t->rear->next = p;
    t->rear = p;
}

/* Removes and returns the passenger at the front of the waiting queue. */
Passenger *dequeue(Train *t) {
    Passenger *temp = t->front;
    if (temp == NULL) return NULL;
    t->front = temp->next;
    if (t->front == NULL) t->rear = NULL;
    temp->next = NULL;
    return temp;
}

/* Undo helper: takes a promoted passenger out of the confirmed list
   and puts them back at the FRONT of the waiting queue. */
void sendBackToQueue(Train *t, int pnr) {
    Passenger *p = deletePassenger(t, pnr);
    if (p == NULL) return;
    t->seats[p->coach - 1][p->seat - 1] = 0;
    p->coach = 0;
    p->seat = 0;
    p->next = t->front;
    t->front = p;
    if (t->rear == NULL) t->rear = p;
}

/* ======================= STACK (undo) ======================= */
/* Pushes a booking (type 1) or cancellation (type 2) onto the undo stack. */
void push(int type, int trainNo, Passenger *p, int promotedPnr) {
    Undo *newNode = malloc(sizeof(Undo));
    if (!newNode) return;
    newNode->type = type;
    newNode->trainNo = trainNo;
    newNode->data = *p;                     /* save a copy of the ticket */
    newNode->promotedPnr = promotedPnr;
    newNode->next = top;
    top = newNode;
}

/* Removes and returns the last operation (NULL if the stack is empty). */
Undo *pop() {
    Undo *temp = top;
    if (temp != NULL) top = temp->next;
    return temp;
}

/* ======================= BINARY SEARCH TREE (trains) ======================= */
/* Creates a train with an empty route, free seats, 5 coaches and empty lists. */
Train *createTrain(int number, char name[]) {
    Train *t = malloc(sizeof(Train));
    if (!t) exit(1);
    t->number = number;
    strcpy(t->name, name);
    for (int c = 0; c < 5; c++)
        for (int s = 0; s < 10; s++)
            t->seats[c][s] = 0;
    t->currentCoach = createCoaches();
    t->head = t->tail = NULL;
    t->passengers = t->front = t->rear = NULL;
    t->left = t->right = NULL;
    return t;
}

/* Inserts a train into the BST: smaller numbers go left, bigger go right. */
Train *insertTrain(Train *node, Train *t) {
    if (node == NULL) return t;
    if (t->number < node->number) node->left = insertTrain(node->left, t);
    else node->right = insertTrain(node->right, t);
    return node;
}

/* Searches the BST for a train number. Returns NULL if not found. */
Train *searchTrain(Train *node, int number) {
    if (node == NULL || node->number == number) return node;
    if (number < node->number) return searchTrain(node->left, number);
    return searchTrain(node->right, number);
}

/* Inorder traversal: prints all trains sorted by train number. */
void displayTrains(Train *node) {
    if (node == NULL) return;
    displayTrains(node->left);
    printf("%d  %-20s  ", node->number, node->name);
    if (node->head != NULL) printf("%s -> %s", node->head->name, node->tail->name);
    printf("  (free seats: %d)\n", countFreeSeats(node));
    displayTrains(node->right);
}

/* Visits every train in the BST and returns the one that has this PNR. */
Train *findTrainOfPnr(Train *node, int pnr) {
    if (node == NULL) return NULL;
    if (searchPnr(node->passengers, pnr) != NULL || searchPnr(node->front, pnr) != NULL)
        return node;
    Train *found = findTrainOfPnr(node->left, pnr);
    if (found == NULL) found = findTrainOfPnr(node->right, pnr);
    return found;
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

/* ======================= MENUS ======================= */
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
