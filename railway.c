/*
 * railway.c
 * ---------
 * Central controller: login, Admin menu, Passenger menu, and the
 * workflows that combine several data structures:
 *
 *   Booking      : BST -> Route DLL -> Seat Array + Coach CLL -> SLL -> Stack
 *   Cancellation : SLL -> Seat Array -> Queue (promotion) -> SLL -> Stack
 *   Undo         : Stack -> BST -> SLL / Seat Array / Queue
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "railway.h"

#define ADMIN_USERNAME    "admin"
#define ADMIN_PASSWORD    "1234"
#define MAX_LOGIN_TRIES   3
#define FIRST_PNR         100001
#define BASE_FARE         250.0f
#define FARE_PER_SEGMENT  240.0f
#define MIN_TRAIN_NUMBER  10000
#define MAX_TRAIN_NUMBER  99999
#define MIN_AGE           1
#define MAX_AGE           120
#define LINE "================================================"

static TrainNode *trainRoot = NULL;   /* BST of all trains      */
static UndoStack undoStack;           /* booking/cancel history */
static int nextPnr = FIRST_PNR;       /* unique PNR generator   */
static int systemLoaded = 0;

/* ============================ Input helpers ============================ */

static void copyText(char *dest, const char *src, int size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

static int isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void trim(char *s)
{
    int start = 0, end = (int)strlen(s);

    while (end > 0 && isSpace(s[end - 1]))
        end--;
    s[end] = '\0';
    while (s[start] != '\0' && isSpace(s[start]))
        start++;
    if (start > 0)
        memmove(s, s + start, (size_t)(end - start + 1));
}

/* Reads one full line. On end of input the program shuts down cleanly. */
static void readLine(const char *prompt, char *buf, int size)
{
    int ch;

    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, size, stdin) == NULL)
    {
        printf("\n\nInput ended. Freeing memory and exiting...\n");
        shutdownRailwaySystem();
        exit(0);
    }
    if (strchr(buf, '\n') == NULL)
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; /* discard the rest of an over-long line */
    trim(buf);
}

/* 1 = valid integer read, 0 = invalid (message already printed). */
static int readInt(const char *prompt, int *out)
{
    char buf[64], *end;
    long value;

    readLine(prompt, buf, sizeof(buf));
    value = strtol(buf, &end, 10);
    if (buf[0] == '\0' || *end != '\0' || value < -1000000000L || value > 1000000000L)
    {
        printf("Invalid input. Please enter a number.\n");
        return 0;
    }
    *out = (int)value;
    return 1;
}

/* Keeps asking until a non-empty line is entered. */
static void readNonEmpty(const char *prompt, char *buf, int size)
{
    for (readLine(prompt, buf, size); buf[0] == '\0'; readLine(prompt, buf, size))
        printf("Input cannot be empty. Please try again.\n");
}

static int readYesNo(const char *prompt)
{
    char buf[16];
    for (;;)
    {
        readLine(prompt, buf, sizeof(buf));
        if (namesEqual(buf, "y") || namesEqual(buf, "yes"))
            return 1;
        if (namesEqual(buf, "n") || namesEqual(buf, "no"))
            return 0;
        printf("Please enter Y or N.\n");
    }
}

/* =========================== Display helpers =========================== */

static void printHeader(const char *title)
{
    int pad = (48 - (int)strlen(title)) / 2;
    printf("\n%s\n%*s%s\n%s\n", LINE, pad < 0 ? 0 : pad, "", title, LINE);
}

/*
 * Shows a menu until its last option (Logout/Exit) is chosen.
 * Option i runs actions[i - 1]; 'count' is the number of actions.
 */
static void runMenu(const char *title, const char *options,
                    void (*const actions[])(void), int count)
{
    int choice;
    for (;;)
    {
        printHeader(title);
        printf("%s", options);
        if (!readInt("Enter choice: ", &choice))
            continue;
        if (choice == count + 1)
            return;
        if (choice >= 1 && choice <= count)
            actions[choice - 1]();
        else
            printf("Invalid choice. Please select 1-%d.\n", count + 1);
    }
}

static void showTrainDetails(TrainNode *t)
{
    printf("\nTrain Number : %d\nTrain Name   : %s\nSource       : %s\nDestination  : %s\n",
           t->trainNumber, t->trainName, t->source, t->destination);
    printf("Route        : ");
    displayRouteInline(&t->route);
    printf("Stops        : %d\nSeats        : %d / %d available\nWaiting List : %d\n",
           t->route.count, countAvailableSeats(t->seats), TOTAL_SEATS, t->waitingList.size);
}

/* Prints a confirmed ticket (wlPosition = 0) or a waitlisted one (no coach/seat). */
static void printTicket(int pnr, const char *name, int age, const char *gender, TrainNode *t,
                        const char *source, const char *destination,
                        int coach, int seat, float fare, int wlPosition)
{
    printHeader("TICKET");
    printf("\nPNR          : %d\nPassenger    : %s\nAge          : %d\nGender       : %s\n\n",
           pnr, name, age, gender);
    printf("Train No.    : %d\nTrain Name   : %s\n\nFrom         : %s\nTo           : %s\n\n",
           t->trainNumber, t->trainName, source, destination);
    if (wlPosition == 0)
        printf("Coach        : C%d\nSeat         : %02d\n\n", coach, seat);
    printf("Fare         : Rs. %.0f\n\n", fare);
    if (wlPosition == 0)
        printf("Status       : CONFIRMED\n\n%s\n", LINE);
    else
        printf("Status       : WAITING LIST (WL %d)\n\n%s\n", wlPosition, LINE);
}

static void printConfirmedTicket(const Passenger *p, TrainNode *t)
{
    printTicket(p->pnr, p->name, p->age, p->gender, t, p->source, p->destination,
                p->coachNumber, p->seatNumber, p->fare, 0);
}

/* ===================== Record <-> node conversions ===================== */

static void passengerToRecord(const Passenger *p, TicketRecord *r)
{
    r->pnr = p->pnr;
    r->age = p->age;
    r->trainNumber = p->trainNumber;
    r->coachNumber = p->coachNumber;
    r->seatNumber = p->seatNumber;
    r->fare = p->fare;
    copyText(r->name, p->name, NAME_LEN);
    copyText(r->gender, p->gender, GENDER_LEN);
    copyText(r->source, p->source, NAME_LEN);
    copyText(r->destination, p->destination, NAME_LEN);
}

static void passengerToWaiting(const Passenger *p, WaitingPassenger *w)
{
    w->pnr = p->pnr;
    w->age = p->age;
    w->fare = p->fare;
    w->next = NULL;
    copyText(w->name, p->name, NAME_LEN);
    copyText(w->gender, p->gender, GENDER_LEN);
    copyText(w->source, p->source, NAME_LEN);
    copyText(w->destination, p->destination, NAME_LEN);
}

/* ==================== Core multi-structure helpers ===================== */

/* Fare grows with the number of route segments travelled (DLL positions). */
static float calculateFare(const Route *route, const char *source, const char *destination)
{
    int segments = stationIndex(route, destination) - stationIndex(route, source);
    return BASE_FARE + FARE_PER_SEGMENT * (float)segments;
}

/*
 * Seat allocation using the CIRCULAR coach list + seat ARRAY.
 * Starts at the train's rotation pointer and walks round the circle once.
 * After a successful allocation the pointer moves to the next coach, so
 * consecutive bookings go C1, C2, C3, C4, C5, C1, ...
 */
static int allocateSeat(TrainNode *t, int *coachNumber, int *seatNumber)
{
    Coach *c = t->nextCoach;
    int i, seat, total = countCoaches(t->coaches);

    for (i = 0; i < total; i++, c = getNextCoach(c))
    {
        seat = findAvailableSeatInCoach(t->seats, c->coachNumber);
        if (seat != 0)
        {
            *coachNumber = c->coachNumber;
            *seatNumber = seat;
            t->nextCoach = getNextCoach(c);
            return 1;
        }
    }
    return 0;
}

/* Finds which train holds a confirmed PNR (walks the BST, searches each SLL). */
static TrainNode *findConfirmedTicket(TrainNode *root, int pnr, Passenger **out)
{
    TrainNode *found;

    if (root == NULL)
        return NULL;
    if ((*out = searchPassenger(root->passengers, pnr)) != NULL)
        return root;
    if ((found = findConfirmedTicket(root->left, pnr, out)) != NULL)
        return found;
    return findConfirmedTicket(root->right, pnr, out);
}

/* Finds which train holds a waitlisted PNR. */
static TrainNode *findWaitingTicket(TrainNode *root, int pnr, int *position)
{
    TrainNode *found;

    if (root == NULL)
        return NULL;
    if ((*position = queuePosition(&root->waitingList, pnr)) != 0)
        return root;
    if ((found = findWaitingTicket(root->left, pnr, position)) != NULL)
        return found;
    return findWaitingTicket(root->right, pnr, position);
}

/*
 * QUEUE -> ARRAY -> SLL : moves the FRONT waiting passenger into a free seat.
 * Fills *promoted (if not NULL) with the new ticket. Returns 1 if promoted.
 */
static int promoteFromWaitingList(TrainNode *t, TicketRecord *promoted)
{
    WaitingPassenger wp;
    Passenger *p = NULL;
    int coach, seat;

    printf("\nChecking Waiting List...\n");
    if (isQueueEmpty(&t->waitingList))
    {
        printf("Waiting list is empty. No passenger to promote.\n");
        return 0;
    }
    if (countAvailableSeats(t->seats) == 0)
        return 0;

    dequeue(&t->waitingList, &wp);
    if (allocateSeat(t, &coach, &seat))
        p = createPassenger(wp.pnr, wp.name, wp.age, wp.gender, t->trainNumber,
                            coach, seat, wp.source, wp.destination, wp.fare);
    if (p == NULL)
    {
        restoreFront(&t->waitingList, &wp); /* put them back where they were */
        return 0;
    }
    bookSeat(t->seats, coach, seat);
    addPassenger(&t->passengers, p);

    printf("\nWaiting passenger found!\n\n%s (PNR %d) has been automatically confirmed.\n"
           "Allotted seat: C%d-%02d\n", p->name, p->pnr, coach, seat);
    if (promoted != NULL)
        passengerToRecord(p, promoted);
    return 1;
}

/* Reads a train number and searches the BST. Prints "Train not found." on miss. */
static TrainNode *promptForTrain(void)
{
    int number;
    TrainNode *t;

    if (!readInt("Enter Train Number: ", &number))
        return NULL;
    if ((t = searchTrain(trainRoot, number)) == NULL)
        printf("Train not found.\n");
    return t;
}

/* ========================= Shared menu actions ========================= */

static void searchTrainAction(void)
{
    TrainNode *t;
    printHeader("SEARCH TRAIN");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("\nTrain Found!\n");
    showTrainDetails(t);
}

static void displayAllTrainsAction(void)
{
    printHeader("ALL TRAINS (BST INORDER)");
    if (trainRoot == NULL)
    {
        printf("No trains available.\n");
        return;
    }
    inorderTrains(trainRoot);
    printf("\nTotal trains: %d\n", countTrains(trainRoot));
}

static void viewRouteAction(void)
{
    TrainNode *t;
    printHeader("TRAIN ROUTE");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("\n%d - %s\n\nForward:\n\n", t->trainNumber, t->trainName);
    displayRouteForward(&t->route);
    printf("\nReverse:\n\n");
    displayRouteReverse(&t->route);
}

static void seatStatusAction(int showPassengers)
{
    TrainNode *t;
    printHeader("SEAT STATUS");
    if ((t = promptForTrain()) == NULL)
        return;

    printf("\n%d - %s\n\nCoaches (circular): ", t->trainNumber, t->trainName);
    displayCoaches(t->coaches);
    printf("Next allocation starts at coach C%d\n\n", t->nextCoach->coachNumber);
    displaySeats(t->seats);
    printf("Waiting list size: %d\n", t->waitingList.size);

    if (showPassengers)
    {
        printf("\nConfirmed passengers (%d):\n\n", countPassengers(t->passengers));
        displayPassengers(t->passengers);
        printf("\nWaiting list:\n\n");
        displayQueue(&t->waitingList);
    }
}

static void adminSeatStatus(void)     { seatStatusAction(1); }
static void passengerSeatStatus(void) { seatStatusAction(0); }

/* ============================ ADMIN actions ============================ */

static void adminAddTrain(void)
{
    int number, inserted = 0;
    char name[NAME_LEN], source[NAME_LEN], destination[NAME_LEN];
    TrainNode *t;

    printHeader("ADD TRAIN");
    if (!readInt("Enter Train Number: ", &number))
        return;
    if (number < MIN_TRAIN_NUMBER || number > MAX_TRAIN_NUMBER)
    {
        printf("Train number must be a 5-digit number (%d-%d).\n", MIN_TRAIN_NUMBER, MAX_TRAIN_NUMBER);
        return;
    }
    if (searchTrain(trainRoot, number) != NULL)
    {
        printf("Duplicate train number. Train %d already exists.\n", number);
        return;
    }

    readNonEmpty("Enter Train Name: ", name, NAME_LEN);
    readNonEmpty("Enter Source: ", source, NAME_LEN);
    for (readNonEmpty("Enter Destination: ", destination, NAME_LEN); namesEqual(source, destination);
         readNonEmpty("Enter Destination: ", destination, NAME_LEN))
        printf("Destination must be different from source.\n");

    if ((t = createTrain(number, name, source, destination)) == NULL)
    {
        printf("Could not create train (memory error).\n");
        return;
    }
    trainRoot = insertTrain(trainRoot, t, &inserted);
    if (!inserted)
    {
        freeTrain(t);
        printf("Duplicate train number.\n");
        return;
    }

    printf("\nTrain added successfully!\nRoute created   : ");
    displayRouteInline(&t->route);
    printf("Coaches created : ");
    displayCoaches(t->coaches);
    printf("Seats created   : %d (all available)\n"
           "Use 'Add Station to Route' to add intermediate stops.\n", TOTAL_SEATS);
}

static void adminDeleteTrain(void)
{
    int deleted = 0;
    TrainNode *t;

    printHeader("DELETE TRAIN");
    if ((t = promptForTrain()) == NULL)
        return;

    printf("\n%d - %s (%s -> %s)\n", t->trainNumber, t->trainName, t->source, t->destination);
    if (t->passengers != NULL || !isQueueEmpty(&t->waitingList))
        printf("WARNING: %d confirmed and %d waiting reservations will be removed.\n",
               countPassengers(t->passengers), t->waitingList.size);
    if (!readYesNo("Are you sure you want to delete this train? (Y/N): "))
    {
        printf("Deletion cancelled.\n");
        return;
    }
    trainRoot = deleteTrain(trainRoot, t->trainNumber, &deleted);
    printf(deleted ? "Train deleted successfully.\n" : "Train not found.\n");
}

static void adminAddStation(void)
{
    TrainNode *t;
    char name[NAME_LEN], after[NAME_LEN];
    Station *pos;
    int result;

    printHeader("ADD STATION TO ROUTE");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("Current route: ");
    displayRouteInline(&t->route);

    readNonEmpty("Enter Station Name: ", name, NAME_LEN);
    if (searchStation(&t->route, name) != NULL)
    {
        printf("Duplicate station. %s is already on this route.\n", name);
        return;
    }

    printf("Insert after which station? (press Enter to add just before %s): ", t->route.tail->name);
    readLine("", after, NAME_LEN);
    if (after[0] == '\0')
        result = addStationBeforeLast(&t->route, name);
    else
    {
        if ((pos = searchStation(&t->route, after)) == NULL)
        {
            printf("Invalid station. %s is not on this route.\n", after);
            return;
        }
        if (pos == t->route.tail)
        {
            printf("Cannot add a station after the final destination (%s).\n", pos->name);
            return;
        }
        result = addStationAfter(&t->route, after, name);
    }

    if (result == ROUTE_OK)
    {
        printf("Station added successfully.\n\nUpdated route:\n");
        displayRouteInline(&t->route);
    }
    else
        printf(result == ROUTE_DUPLICATE ? "Duplicate station.\n" : "Could not add station.\n");
}

static void adminDeleteStation(void)
{
    TrainNode *t;
    char name[NAME_LEN];
    Station *s;

    printHeader("DELETE STATION FROM ROUTE");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("Current route: ");
    displayRouteInline(&t->route);
    readNonEmpty("Enter Station Name: ", name, NAME_LEN);

    if ((s = searchStation(&t->route, name)) == NULL)
        printf("Station not found. %s is not on this route.\n", name);
    else if (s == t->route.head || s == t->route.tail)
        printf("Cannot delete %s: it is the source/destination of the train.\n", s->name);
    else if (passengerUsesStation(t->passengers, s->name) || queueUsesStation(&t->waitingList, s->name))
        printf("Cannot delete %s: passengers are booked to/from this station.\n", s->name);
    else
    {
        deleteStation(&t->route, s->name);
        printf("Station deleted successfully.\n\nUpdated route:\n");
        displayRouteInline(&t->route);
    }
}

static void adminDisplayRoute(void)
{
    TrainNode *t;
    char name[NAME_LEN];
    int choice, index;

    printHeader("DISPLAY TRAIN ROUTE");
    if ((t = promptForTrain()) == NULL)
        return;

    for (;;)
    {
        printf("\n%d - %s\n1. Display Forward Route\n2. Display Reverse Route\n"
               "3. Search Station\n4. Back\n", t->trainNumber, t->trainName);
        if (!readInt("Enter choice: ", &choice))
            continue;
        if (choice == 1)
        {
            printf("\nForward:\n\n");
            displayRouteForward(&t->route);
        }
        else if (choice == 2)
        {
            printf("\nReverse:\n\n");
            displayRouteReverse(&t->route);
        }
        else if (choice == 3)
        {
            readNonEmpty("Enter Station Name: ", name, NAME_LEN);
            if ((index = stationIndex(&t->route, name)) < 0)
                printf("Station %s is NOT on this route.\n", name);
            else
                printf("Station found: %s is stop %d of %d.\n",
                       searchStation(&t->route, name)->name, index + 1, t->route.count);
        }
        else if (choice == 4)
            return;
        else
            printf("Invalid choice. Please select 1-4.\n");
    }
}

static int adminLogin(void)
{
    char username[NAME_LEN], password[NAME_LEN];
    int attempt;

    printHeader("ADMIN LOGIN");
    for (attempt = 1; attempt <= MAX_LOGIN_TRIES; attempt++)
    {
        readLine("Username: ", username, NAME_LEN);
        readLine("Password: ", password, NAME_LEN);
        if (strcmp(username, ADMIN_USERNAME) == 0 && strcmp(password, ADMIN_PASSWORD) == 0)
        {
            printf("\nLogin successful. Welcome, Admin!\n");
            return 1;
        }
        printf("Invalid username or password. (%d attempt(s) left)\n", MAX_LOGIN_TRIES - attempt);
    }
    printf("Too many failed attempts. Returning to main menu.\n");
    return 0;
}

static void adminMenu(void)
{
    static void (*const actions[])(void) = {
        adminAddTrain, adminDeleteTrain, searchTrainAction, displayAllTrainsAction,
        adminAddStation, adminDeleteStation, adminDisplayRoute, adminSeatStatus};

    if (!adminLogin())
        return;
    runMenu("ADMIN MENU",
            "\n1. Add Train\n2. Delete Train\n3. Search Train\n4. Display All Trains\n"
            "5. Add Station to Route\n6. Delete Station from Route\n7. Display Train Route\n"
            "8. Display Seat Status\n9. Logout\n\n",
            actions, 8);
    printf("Admin logged out.\n");
}

/* ========================== PASSENGER actions ========================== */

/* Asks for a station until it is on the route; "0" aborts. Returns the DLL node. */
static Station *promptStation(TrainNode *t, const char *prompt)
{
    char name[NAME_LEN];
    Station *s;

    for (;;)
    {
        readNonEmpty(prompt, name, NAME_LEN);
        if (strcmp(name, "0") == 0)
            return NULL;
        if ((s = searchStation(&t->route, name)) != NULL)
            return s;
        printf("Invalid station. %s is not on the route of train %d. (enter 0 to cancel)\n",
               name, t->trainNumber);
    }
}

static void bookTicketAction(void)
{
    TrainNode *t;
    Station *src, *dst;
    char name[NAME_LEN], gender[GENDER_LEN];
    int age, genderChoice, coach, seat, choice;
    float fare;
    Passenger *p;
    TicketRecord record;

    /* Step 1: BST search */
    printHeader("BOOK TICKET");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("\nTrain Found: %d - %s\nRoute: ", t->trainNumber, t->trainName);
    displayRouteInline(&t->route);
    printf("Seats available: %d / %d\n\n", countAvailableSeats(t->seats), TOTAL_SEATS);

    /* Step 2: passenger details */
    readNonEmpty("Passenger Name: ", name, NAME_LEN);
    for (;;)
    {
        if (readInt("Age: ", &age))
        {
            if (age >= MIN_AGE && age <= MAX_AGE)
                break;
            printf("Age must be between %d and %d.\n", MIN_AGE, MAX_AGE);
        }
    }
    for (;;)
    {
        printf("Gender (1. Male  2. Female  3. Other)\n");
        if (readInt("Enter choice: ", &genderChoice) && genderChoice >= 1 && genderChoice <= 3)
            break;
        printf("Invalid gender choice.\n");
    }
    copyText(gender, genderChoice == 1 ? "Male" : genderChoice == 2 ? "Female" : "Other", GENDER_LEN);

    /* Step 3: validate source/destination with the route DLL */
    for (;;)
    {
        if ((src = promptStation(t, "Source: ")) == NULL ||
            (dst = promptStation(t, "Destination: ")) == NULL)
        {
            printf("Booking cancelled.\n");
            return;
        }
        if (src == dst)
            printf("Invalid source/destination: they cannot be the same station.\n");
        else if (stationIndex(&t->route, src->name) > stationIndex(&t->route, dst->name))
            printf("Invalid source/destination: train %d runs %s -> %s, "
                   "so you cannot travel from %s back to %s.\n",
                   t->trainNumber, t->source, t->destination, src->name, dst->name);
        else
            break;
    }
    fare = calculateFare(&t->route, src->name, dst->name);

    /* Step 4: seat array full? -> offer waiting queue */
    if (countAvailableSeats(t->seats) == 0)
    {
        printf("\nAll seats are currently booked.\n\n"
               "Would you like to join the waiting list?\n\n1. Yes\n2. No\n\n");
        if (!readInt("Enter choice: ", &choice) || choice != 1)
        {
            printf("Booking not done.\n");
            return;
        }
        if (!enqueue(&t->waitingList, nextPnr, name, age, gender, src->name, dst->name, fare))
            return;
        printHeader("ADDED TO WAITING LIST");
        printf("\nPNR Number : %d\nPassenger  : %s\nTrain      : %d - %s\n", nextPnr++, name,
               t->trainNumber, t->trainName);
        printf("From       : %s\nTo         : %s\nStatus     : WL %d\nFare       : Rs. %.0f\n\n",
               src->name, dst->name, t->waitingList.size, fare);
        printf("You will be confirmed automatically when a seat is cancelled.\n%s\n", LINE);
        return;
    }

    /* Step 5: circular coach list + seat array -> find a seat */
    if (!allocateSeat(t, &coach, &seat))
    {
        printf("No seats available.\n");
        return;
    }

    /* Step 6: PNR + SLL node, Step 7: mark seat booked in the array */
    p = createPassenger(nextPnr, name, age, gender, t->trainNumber, coach, seat,
                        src->name, dst->name, fare);
    if (p == NULL)
    {
        printf("Booking failed (memory error).\n");
        return;
    }
    nextPnr++;
    addPassenger(&t->passengers, p);
    bookSeat(t->seats, coach, seat);

    /* Step 8: push BOOK onto the undo stack */
    passengerToRecord(p, &record);
    if (!push(&undoStack, OP_BOOK, &record, NULL))
        printf("Warning: this booking cannot be undone (memory error).\n");

    /* Step 9: ticket */
    printHeader("BOOKING CONFIRMED");
    printf("\nPNR Number : %d\nPassenger  : %s\nTrain      : %d\nTrain Name : %s\n",
           p->pnr, p->name, p->trainNumber, t->trainName);
    printf("From       : %s\nTo         : %s\nCoach      : C%d\nSeat       : %02d\n",
           p->source, p->destination, p->coachNumber, p->seatNumber);
    printf("Fare       : Rs. %.0f\n\n%s\n", p->fare, LINE);
}

static void cancelTicketAction(void)
{
    int pnr, position = 0, promoted;
    TrainNode *t;
    Passenger *p;
    TicketRecord cancelled, promotedRecord;

    printHeader("CANCEL TICKET");
    if (!readInt("Enter PNR Number: ", &pnr))
        return;

    if ((t = findConfirmedTicket(trainRoot, pnr, &p)) == NULL)
    {
        if ((t = findWaitingTicket(trainRoot, pnr, &position)) != NULL)
            printf("PNR %d is on the waiting list of train %d (WL %d).\n"
                   "Only confirmed tickets can be cancelled; it will be confirmed "
                   "automatically when a seat frees up.\n", pnr, t->trainNumber, position);
        else
            printf("Invalid PNR. No ticket found with PNR %d.\n", pnr);
        return;
    }

    /* 1. Display ticket */
    printConfirmedTicket(p, t);
    if (!readYesNo("Confirm cancellation? (Y/N): "))
    {
        printf("Cancellation aborted.\n");
        return;
    }

    /* 2. Remove from SLL (keep a snapshot for the stack)  3. Release the seat */
    passengerToRecord(p, &cancelled);
    free(removePassenger(&t->passengers, pnr));
    releaseSeat(t->seats, cancelled.coachNumber, cancelled.seatNumber);
    printf("\nCancellation successful.\n\nSeat C%d-%02d is now available.\n",
           cancelled.coachNumber, cancelled.seatNumber);

    /* 4. Queue: promote first waiting passenger  5. Push CANCEL (with promotion info) */
    promoted = promoteFromWaitingList(t, &promotedRecord);
    if (!push(&undoStack, OP_CANCEL, &cancelled, promoted ? &promotedRecord : NULL))
        printf("Warning: this cancellation cannot be undone (memory error).\n");
}

static void viewMyTicketAction(void)
{
    int pnr, position = 0;
    TrainNode *t;
    Passenger *p;
    WaitingPassenger *w;

    printHeader("VIEW MY TICKET");
    if (!readInt("Enter PNR Number: ", &pnr))
        return;

    if ((t = findConfirmedTicket(trainRoot, pnr, &p)) != NULL)
        printConfirmedTicket(p, t);
    else if ((t = findWaitingTicket(trainRoot, pnr, &position)) != NULL)
    {
        w = searchQueue(&t->waitingList, pnr);
        printTicket(w->pnr, w->name, w->age, w->gender, t, w->source, w->destination,
                    0, 0, w->fare, position);
    }
    else
        printf("Invalid PNR. No ticket found with PNR %d.\n", pnr);
}

static void viewWaitingListAction(void)
{
    TrainNode *t;
    WaitingPassenger *front;

    printHeader("WAITING LIST");
    if ((t = promptForTrain()) == NULL)
        return;
    printf("\n%d - %s\n\n", t->trainNumber, t->trainName);
    displayQueue(&t->waitingList);
    if ((front = peekQueue(&t->waitingList)) != NULL)
        printf("\nNext to be confirmed (peek): %s, PNR %d\n", front->name, front->pnr);
}

/* ----------------------------- Undo helpers ----------------------------- */

static void undoBooking(TrainNode *t, const Operation *op)
{
    Passenger *p = removePassenger(&t->passengers, op->ticket.pnr);
    if (p == NULL)
    {
        printf("PNR %d is no longer active. Nothing to undo; record discarded.\n", op->ticket.pnr);
        return;
    }
    releaseSeat(t->seats, p->coachNumber, p->seatNumber);
    printf("\nUndo successful!\n\nBooking of %s (PNR %d) has been removed.\n"
           "Seat C%d-%02d has been released.\n", p->name, p->pnr, p->coachNumber, p->seatNumber);
    free(p);

    /* A seat just became free; serve the waiting list if anyone is waiting. */
    if (!isQueueEmpty(&t->waitingList))
        promoteFromWaitingList(t, NULL);
}

static void undoCancellation(TrainNode *t, const Operation *op)
{
    const TicketRecord *r = &op->ticket;
    Passenger *p, *promotedPassenger;
    WaitingPassenger back;
    int coach = r->coachNumber, seat = r->seatNumber;

    if (searchPassenger(t->passengers, r->pnr) != NULL)
    {
        printf("PNR %d is already active. Nothing to undo; record discarded.\n", r->pnr);
        return;
    }

    /* Reverse the automatic promotion: SLL -> back to FRONT of the queue. */
    if (op->hasPromotion &&
        (promotedPassenger = removePassenger(&t->passengers, op->promoted.pnr)) != NULL)
    {
        releaseSeat(t->seats, promotedPassenger->coachNumber, promotedPassenger->seatNumber);
        passengerToWaiting(promotedPassenger, &back);
        if (restoreFront(&t->waitingList, &back))
            printf("%s (PNR %d) moved back to the front of the waiting list.\n",
                   promotedPassenger->name, promotedPassenger->pnr);
        free(promotedPassenger);
    }

    /* Restore the original seat if possible, otherwise any free seat. */
    if (!isSeatAvailable(t->seats, coach, seat) && !allocateSeat(t, &coach, &seat))
    {
        printf("Undo failed: no seat is available to restore PNR %d.\n", r->pnr);
        return;
    }
    p = createPassenger(r->pnr, r->name, r->age, r->gender, r->trainNumber,
                        coach, seat, r->source, r->destination, r->fare);
    if (p == NULL)
    {
        printf("Undo failed (memory error).\n");
        return;
    }
    bookSeat(t->seats, coach, seat);
    addPassenger(&t->passengers, p);
    printf("\nUndo successful!\n\n%s's ticket has been restored.\n"
           "Seat C%d-%02d has been restored.\n", p->name, coach, seat);
}

static void undoLastOperationAction(void)
{
    Operation *top;
    Operation op;
    TrainNode *t;
    char prompt[64];

    printHeader("UNDO OPERATION");
    if (isStackEmpty(&undoStack))
    {
        printf("Undo stack is empty. Nothing to undo.\n");
        return;
    }
    printf("Operation history:\n\n");
    displayStack(&undoStack);

    top = peek(&undoStack);
    printf("\nLast Operation:\n%s TICKET\n\nPNR       : %d\nPassenger : %s\n", operationName(top->type),
           top->ticket.pnr, top->ticket.name);
    printf("Train     : %d\nCoach     : C%d\nSeat      : %02d\n",
           top->ticket.trainNumber, top->ticket.coachNumber, top->ticket.seatNumber);
    if (top->hasPromotion)
        printf("Promoted  : %s (PNR %d) from waiting list\n", top->promoted.name, top->promoted.pnr);

    sprintf(prompt, "\nUndo %s? (Y/N): ", top->type == OP_BOOK ? "booking" : "cancellation");
    if (!readYesNo(prompt))
    {
        printf("Undo cancelled. The operation stays on the stack.\n");
        return;
    }

    pop(&undoStack, &op);
    if ((t = searchTrain(trainRoot, op.ticket.trainNumber)) == NULL)
        printf("Train %d no longer exists. Operation cannot be undone; record discarded.\n",
               op.ticket.trainNumber);
    else if (op.type == OP_BOOK)
        undoBooking(t, &op);
    else
        undoCancellation(t, &op);
}

static void passengerMenu(void)
{
    static void (*const actions[])(void) = {
        searchTrainAction, viewRouteAction, passengerSeatStatus, bookTicketAction,
        cancelTicketAction, viewMyTicketAction, viewWaitingListAction,
        undoLastOperationAction, displayAllTrainsAction};
    char name[NAME_LEN];

    printHeader("PASSENGER LOGIN");
    readNonEmpty("Enter your name: ", name, NAME_LEN);
    printf("\nWelcome, %s!\n", name);
    runMenu("PASSENGER MENU",
            "\n1. Search Train\n2. View Train Route\n3. Check Seat Availability\n4. Book Ticket\n"
            "5. Cancel Ticket\n6. View My Ticket\n7. View Waiting List\n8. Undo Last Operation\n"
            "9. Display All Trains\n10. Logout\n\n",
            actions, 9);
    printf("Goodbye, %s! Logged out.\n", name);
}

/* ========================== System life-cycle ========================== */

/* Creates one train, adds its intermediate stations, inserts into the BST. */
static void preloadTrain(int number, const char *name, const char *stations[], int count)
{
    TrainNode *t;
    int i, inserted = 0;

    if ((t = createTrain(number, name, stations[0], stations[count - 1])) == NULL)
        return;
    for (i = 1; i < count - 1; i++)
        addStationBeforeLast(&t->route, stations[i]);
    trainRoot = insertTrain(trainRoot, t, &inserted);
    if (!inserted)
        freeTrain(t);
}

void loadInitialData(void)
{
    const char *route12951[] = {"Mumbai", "Surat", "Vadodara", "Ahmedabad", "Kota", "Delhi"};
    const char *route12952[] = {"Delhi", "Kota", "Ahmedabad", "Vadodara", "Surat", "Mumbai"};
    const char *route12431[] = {"Delhi", "Mathura", "Kota", "Ratlam", "Vadodara", "Surat", "Mumbai"};

    trainRoot = NULL;
    initStack(&undoStack);
    nextPnr = FIRST_PNR;
    preloadTrain(12951, "Mumbai Rajdhani", route12951, 6);
    preloadTrain(12952, "Delhi Rajdhani", route12952, 6);
    preloadTrain(12431, "Rajdhani Express", route12431, 7);
    systemLoaded = 1;
}

void runRailwaySystem(void)
{
    static void (*const actions[])(void) = {adminMenu, passengerMenu};

    runMenu("RAILWAY RESERVATION SYSTEM", "\n1. Admin Login\n2. Passenger Login\n3. Exit\n\n", actions, 2);
    printf("\nThank you for using the Railway Reservation System!\n");
}

void shutdownRailwaySystem(void)
{
    if (!systemLoaded)
        return;
    freeAllTrains(trainRoot);
    trainRoot = NULL;
    freeStack(&undoStack);
    systemLoaded = 0;
}
