/*
 * railway.c
 * ---------
 * All admin and passenger operations. This is the file that makes the
 * seven data structures work together:
 *
 *   Booking      : BST -> DLL (stations) -> CLL + Array (seat) -> SLL -> Stack
 *   Cancellation : SLL -> Array -> Queue (promotion) -> SLL -> Stack
 *   Undo         : Stack -> BST -> SLL / Array / Queue
 */
#include "railway.h"

struct TrainNode *trainRoot = NULL;     /* root of the BST of all trains */
int nextPnr = FIRST_PNR;                /* next PNR number to give out   */

/* =========================== Basic helpers ============================ */

/* Stops the program if malloc() could not get memory. */
void checkMemory(void *pointer)
{
    if (pointer == NULL)
    {
        printf("Memory allocation failed! Exiting.\n");
        exit(1);
    }
}

/* Frees every train (and its lists) and the undo stack. */
void freeAllData(void)
{
    freeAllTrains(trainRoot);
    trainRoot = NULL;
    freeStack();
}

/* Called when there is no more input (end of file): free memory and exit. */
void stopOnEndOfInput(void)
{
    printf("\n\nInput ended. Freeing memory and exiting...\n");
    freeAllData();
    exit(0);
}

/* Throws away the rest of the current input line. */
void clearInputBuffer(void)
{
    int ch = getchar();
    while (ch != '\n' && ch != EOF)
        ch = getchar();
}

/* Reads a whole number. Returns 1 if valid, 0 if the user typed something else. */
int readInt(int *value)
{
    int result = scanf("%d", value);

    if (result == EOF)
        stopOnEndOfInput();
    clearInputBuffer();

    if (result != 1)
    {
        printf("Invalid input. Please enter a number.\n");
        return 0;
    }
    return 1;
}

/* Reads a line of text (spaces allowed, e.g. "New Delhi"). */
void readText(char text[])
{
    int length;

    if (scanf(" %99[^\n]", text) == EOF)
        stopOnEndOfInput();
    clearInputBuffer();

    length = strlen(text);                  /* remove spaces at the end */
    while (length > 0 && (text[length - 1] == ' ' || text[length - 1] == '\r'))
    {
        text[length - 1] = '\0';
        length--;
    }
}

/* Reads Y or N. Returns 1 for yes, 0 for no. */
int readYesNo(void)
{
    char answer;

    while (1)
    {
        if (scanf(" %c", &answer) == EOF)
            stopOnEndOfInput();
        clearInputBuffer();

        if (answer == 'y' || answer == 'Y')
            return 1;
        if (answer == 'n' || answer == 'N')
            return 0;
        printf("Please enter Y or N: ");
    }
}

void printLine(void)
{
    printf("================================================\n");
}

/* Prints a title centred between two lines. */
void printHeader(char title[])
{
    int i;
    int spaces = (48 - (int)strlen(title)) / 2;

    printf("\n");
    printLine();
    for (i = 0; i < spaces; i++)
        printf(" ");
    printf("%s\n", title);
    printLine();
}

/* Prints the details of one train. */
void printTrainDetails(struct TrainNode *train)
{
    printf("\nTrain Number : %d\n", train->trainNumber);
    printf("Train Name   : %s\n", train->trainName);
    printf("Source       : %s\n", train->source);
    printf("Destination  : %s\n", train->destination);
    printf("Route        : ");
    displayRouteInLine(train);
    printf("Stops        : %d\n", countStations(train));
    printf("Seats        : %d / %d available\n", countFreeSeats(train), TOTAL_SEATS);
    printf("Waiting List : %d\n", countWaiting(train));
}

/* Prints a ticket. A waiting passenger has coachNumber 0 and shows a WL number. */
void printTicket(struct TrainNode *train, struct Passenger *passenger)
{
    printHeader("TICKET");
    printf("\nPNR          : %d\n", passenger->pnr);
    printf("Passenger    : %s\n", passenger->name);
    printf("Age          : %d\n", passenger->age);
    printf("Gender       : %s\n\n", passenger->gender);
    printf("Train No.    : %d\n", train->trainNumber);
    printf("Train Name   : %s\n\n", train->trainName);
    printf("From         : %s\n", passenger->source);
    printf("To           : %s\n\n", passenger->destination);

    if (passenger->coachNumber != 0)
    {
        printf("Coach        : C%d\n", passenger->coachNumber);
        printf("Seat         : %02d\n\n", passenger->seatNumber);
        printf("Fare         : Rs. %.0f\n\n", passenger->fare);
        printf("Status       : CONFIRMED\n\n");
    }
    else
    {
        printf("Fare         : Rs. %.0f\n\n", passenger->fare);
        printf("Status       : WAITING LIST (WL %d)\n\n",
               getWaitingPosition(train, passenger->pnr));
    }
    printLine();
}

/* Asks for a train number and searches the BST. Returns NULL if not found. */
struct TrainNode *askForTrain(void)
{
    int number;
    struct TrainNode *train;

    printf("Enter Train Number: ");
    if (!readInt(&number))
        return NULL;

    train = searchTrain(trainRoot, number);
    if (train == NULL)
        printf("Train not found.\n");
    return train;
}

/* Fare = Rs. 250 + Rs. 240 for every station travelled on the route. */
float calculateFare(struct TrainNode *train, char source[], char destination[])
{
    int stops = getStationPosition(train, destination) - getStationPosition(train, source);
    return BASE_FARE + FARE_PER_STOP * stops;
}

/*
 * Finds a free seat using the CIRCULAR coach list and the seat ARRAY.
 * The search starts at train->currentCoach and goes round the circle once.
 * After a booking, currentCoach moves to the next coach, so bookings go
 * C1, C2, C3, C4, C5, C1, ...
 * Returns 1 and fills coachNumber/seatNumber if a seat was found, else 0.
 */
int allocateSeat(struct TrainNode *train, int *coachNumber, int *seatNumber)
{
    struct Coach *coach = train->currentCoach;
    int i, seat;

    for (i = 0; i < MAX_COACHES; i++)
    {
        seat = findFreeSeat(train, coach->coachNumber);
        if (seat != 0)
        {
            *coachNumber = coach->coachNumber;
            *seatNumber = seat;
            train->currentCoach = getNextCoach(coach);
            return 1;
        }
        coach = getNextCoach(coach);
    }
    return 0;
}

/*
 * Gives a free seat to the FIRST waiting passenger (Queue -> Array -> SLL).
 * The same passenger node is moved from the queue into the confirmed list.
 * Returns the promoted passenger, or NULL if nobody was promoted.
 */
struct Passenger *promoteWaitingPassenger(struct TrainNode *train)
{
    struct Passenger *passenger;
    int coach, seat;

    printf("\nChecking Waiting List...\n");
    if (isQueueEmpty(train))
    {
        printf("Waiting list is empty. No passenger to promote.\n");
        return NULL;
    }
    if (!allocateSeat(train, &coach, &seat))
        return NULL;

    passenger = dequeue(train);
    passenger->coachNumber = coach;
    passenger->seatNumber = seat;
    bookSeat(train, coach, seat);
    addPassenger(train, passenger);

    printf("\nWaiting passenger found!\n\n");
    printf("%s (PNR %d) has been automatically confirmed.\n", passenger->name, passenger->pnr);
    printf("Allotted seat: C%d-%02d\n", coach, seat);
    return passenger;
}

/* Visits every train in the BST to find which one has this confirmed PNR. */
struct TrainNode *findTrainOfPassenger(struct TrainNode *root, int pnr)
{
    struct TrainNode *found;

    if (root == NULL)
        return NULL;
    if (searchPassenger(root, pnr) != NULL)
        return root;

    found = findTrainOfPassenger(root->left, pnr);
    if (found == NULL)
        found = findTrainOfPassenger(root->right, pnr);
    return found;
}

/* Visits every train in the BST to find which one has this PNR on its waiting list. */
struct TrainNode *findTrainOfWaitingPassenger(struct TrainNode *root, int pnr)
{
    struct TrainNode *found;

    if (root == NULL)
        return NULL;
    if (searchWaitingList(root, pnr) != NULL)
        return root;

    found = findTrainOfWaitingPassenger(root->left, pnr);
    if (found == NULL)
        found = findTrainOfWaitingPassenger(root->right, pnr);
    return found;
}

/* Returns 1 if any confirmed or waiting passenger boards or leaves at this station. */
int isStationInUse(struct TrainNode *train, char name[])
{
    struct Passenger *passenger;

    for (passenger = train->passengerHead; passenger != NULL; passenger = passenger->next)
        if (sameName(passenger->source, name) || sameName(passenger->destination, name))
            return 1;

    for (passenger = train->waitFront; passenger != NULL; passenger = passenger->next)
        if (sameName(passenger->source, name) || sameName(passenger->destination, name))
            return 1;

    return 0;
}

/* ============================ ADMIN ============================ */

/* Asks for username and password (3 attempts). Returns 1 on success. */
int adminLogin(void)
{
    char username[NAME_LEN], password[NAME_LEN];
    int attempt;

    printHeader("ADMIN LOGIN");
    for (attempt = 1; attempt <= 3; attempt++)
    {
        printf("Username: ");
        readText(username);
        printf("Password: ");
        readText(password);

        if (strcmp(username, "admin") == 0 && strcmp(password, "1234") == 0)
        {
            printf("\nLogin successful. Welcome, Admin!\n");
            return 1;
        }
        printf("Invalid username or password. (%d attempt(s) left)\n", 3 - attempt);
    }
    printf("Too many failed attempts. Returning to main menu.\n");
    return 0;
}

/* Admin: creates a new train and inserts it into the BST. */
void addTrain(void)
{
    int number;
    char name[NAME_LEN], source[NAME_LEN], destination[NAME_LEN];
    struct TrainNode *train;

    printHeader("ADD TRAIN");
    printf("Enter Train Number: ");
    if (!readInt(&number))
        return;
    if (number < 10000 || number > 99999)
    {
        printf("Train number must be a 5-digit number (10000-99999).\n");
        return;
    }
    if (searchTrain(trainRoot, number) != NULL)
    {
        printf("Duplicate train number. Train %d already exists.\n", number);
        return;
    }

    printf("Enter Train Name: ");
    readText(name);
    printf("Enter Source: ");
    readText(source);
    printf("Enter Destination: ");
    readText(destination);
    while (sameName(source, destination))
    {
        printf("Destination must be different from source.\nEnter Destination: ");
        readText(destination);
    }

    train = createTrain(number, name, source, destination);
    trainRoot = insertTrain(trainRoot, train);

    printf("\nTrain added successfully!\n");
    printf("Route created   : ");
    displayRouteInLine(train);
    printf("Coaches created : ");
    displayCoaches(train);
    printf("Seats created   : %d (all available)\n", TOTAL_SEATS);
    printf("Use 'Add Station to Route' to add intermediate stops.\n");
}

/* Admin: deletes a train from the BST (and frees all of its data). */
void removeTrain(void)
{
    struct TrainNode *train;

    printHeader("DELETE TRAIN");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("\n%d - %s (%s -> %s)\n", train->trainNumber, train->trainName,
           train->source, train->destination);
    if (countPassengers(train) > 0 || countWaiting(train) > 0)
        printf("WARNING: %d confirmed and %d waiting reservations will be removed.\n",
               countPassengers(train), countWaiting(train));

    printf("Are you sure you want to delete this train? (Y/N): ");
    if (!readYesNo())
    {
        printf("Deletion cancelled.\n");
        return;
    }
    trainRoot = deleteTrain(trainRoot, train->trainNumber);
    printf("Train deleted successfully.\n");
}

/* Admin: inserts a new station into a train's route (doubly linked list). */
void addStation(void)
{
    struct TrainNode *train;
    struct Station *position;
    char name[NAME_LEN], afterName[NAME_LEN];

    printHeader("ADD STATION TO ROUTE");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("Current route: ");
    displayRouteInLine(train);
    printf("Enter Station Name: ");
    readText(name);
    if (searchStation(train, name) != NULL)
    {
        printf("Duplicate station. %s is already on this route.\n", name);
        return;
    }

    printf("Insert after which station? (enter 0 to add just before %s): ", train->routeTail->name);
    readText(afterName);

    if (strcmp(afterName, "0") == 0)
        position = train->routeTail->prev;
    else
    {
        position = searchStation(train, afterName);
        if (position == NULL)
        {
            printf("Invalid station. %s is not on this route.\n", afterName);
            return;
        }
        if (position == train->routeTail)
        {
            printf("Cannot add a station after the final destination (%s).\n", position->name);
            return;
        }
    }

    insertStationAfter(train, position, name);
    printf("Station added successfully.\n\nUpdated route:\n");
    displayRouteInLine(train);
}

/* Admin: deletes a station from a train's route. */
void removeStation(void)
{
    struct TrainNode *train;
    struct Station *station;
    char name[NAME_LEN];

    printHeader("DELETE STATION FROM ROUTE");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("Current route: ");
    displayRouteInLine(train);
    printf("Enter Station Name: ");
    readText(name);

    station = searchStation(train, name);
    if (station == NULL)
    {
        printf("Station not found. %s is not on this route.\n", name);
        return;
    }
    if (station == train->routeHead || station == train->routeTail)
    {
        printf("Cannot delete %s: it is the source/destination of the train.\n", station->name);
        return;
    }
    if (isStationInUse(train, station->name))
    {
        printf("Cannot delete %s: passengers are booked to/from this station.\n", station->name);
        return;
    }

    deleteStation(train, station);
    printf("Station deleted successfully.\n\nUpdated route:\n");
    displayRouteInLine(train);
}

/* Admin: shows the route forward/backward or searches a station. */
void showRouteMenu(void)
{
    struct TrainNode *train;
    char name[NAME_LEN];
    int choice, position;

    printHeader("DISPLAY TRAIN ROUTE");
    train = askForTrain();
    if (train == NULL)
        return;

    while (1)
    {
        printf("\n%d - %s\n", train->trainNumber, train->trainName);
        printf("1. Display Forward Route\n");
        printf("2. Display Reverse Route\n");
        printf("3. Search Station\n");
        printf("4. Back\n");
        printf("Enter choice: ");
        if (!readInt(&choice))
            continue;

        switch (choice)
        {
        case 1:
            printf("\nForward:\n\n");
            displayRouteForward(train);
            break;
        case 2:
            printf("\nReverse:\n\n");
            displayRouteBackward(train);
            break;
        case 3:
            printf("Enter Station Name: ");
            readText(name);
            position = getStationPosition(train, name);
            if (position == -1)
                printf("Station %s is NOT on this route.\n", name);
            else
                printf("Station found: %s is stop %d of %d.\n",
                       searchStation(train, name)->name, position + 1, countStations(train));
            break;
        case 4:
            return;
        default:
            printf("Invalid choice. Please select 1-4.\n");
        }
    }
}

/* ===================== Shared by admin and passenger ===================== */

/* Searches a train in the BST and shows its details. */
void showTrain(void)
{
    struct TrainNode *train;

    printHeader("SEARCH TRAIN");
    train = askForTrain();
    if (train == NULL)
        return;
    printf("\nTrain Found!\n");
    printTrainDetails(train);
}

/* Shows all trains in sorted order (BST inorder traversal). */
void showAllTrains(void)
{
    printHeader("ALL TRAINS (BST INORDER)");
    if (trainRoot == NULL)
    {
        printf("No trains available.\n");
        return;
    }
    displayAllTrains(trainRoot);
    printf("\nTotal trains: %d\n", countTrains(trainRoot));
}

/* Shows a train's route forward and backward (doubly linked list). */
void viewRoute(void)
{
    struct TrainNode *train;

    printHeader("TRAIN ROUTE");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("\n%d - %s\n", train->trainNumber, train->trainName);
    printf("\nForward:\n\n");
    displayRouteForward(train);
    printf("\nReverse:\n\n");
    displayRouteBackward(train);
}

/* Shows coaches, seat map, confirmed bookings and the waiting list of a train. */
void showSeatStatus(void)
{
    struct TrainNode *train;

    printHeader("SEAT STATUS");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("\n%d - %s\n\n", train->trainNumber, train->trainName);
    printf("Coaches (circular): ");
    displayCoaches(train);
    printf("Next allocation starts at coach C%d\n\n", train->currentCoach->coachNumber);
    displaySeats(train);
    printf("Waiting list size: %d\n", countWaiting(train));

    printf("\nConfirmed passengers (%d):\n\n", countPassengers(train));
    displayPassengers(train);
    printf("\nWaiting list:\n\n");
    displayWaitingList(train);
}

/* ============================ PASSENGER ============================ */

/* Reads name, age and gender of a passenger. */
void readPassengerDetails(struct Passenger *passenger)
{
    int genderChoice;

    printf("Passenger Name: ");
    readText(passenger->name);

    while (1)
    {
        printf("Age: ");
        if (readInt(&passenger->age))
        {
            if (passenger->age >= MIN_AGE && passenger->age <= MAX_AGE)
                break;
            printf("Age must be between %d and %d.\n", MIN_AGE, MAX_AGE);
        }
    }

    while (1)
    {
        printf("Gender (1. Male  2. Female  3. Other)\nEnter choice: ");
        if (readInt(&genderChoice) && genderChoice >= 1 && genderChoice <= 3)
            break;
        printf("Invalid gender choice.\n");
    }

    if (genderChoice == 1)
        strcpy(passenger->gender, "Male");
    else if (genderChoice == 2)
        strcpy(passenger->gender, "Female");
    else
        strcpy(passenger->gender, "Other");
}

/* Asks for a station until it is on the route. Returns NULL if the user enters 0. */
struct Station *askStation(struct TrainNode *train, char prompt[])
{
    char name[NAME_LEN];
    struct Station *station;

    while (1)
    {
        printf("%s", prompt);
        readText(name);
        if (strcmp(name, "0") == 0)
            return NULL;

        station = searchStation(train, name);
        if (station != NULL)
            return station;
        printf("Invalid station. %s is not on the route of train %d. (enter 0 to cancel)\n",
               name, train->trainNumber);
    }
}

/*
 * Asks for source and destination and checks them with the route (DLL):
 * both must be on the route, different, and in the direction the train runs.
 * Fills source, destination and fare. Returns 0 if the user cancelled.
 */
int askJourney(struct TrainNode *train, struct Passenger *passenger)
{
    struct Station *source, *destination;

    while (1)
    {
        source = askStation(train, "Source: ");
        if (source == NULL)
            return 0;
        destination = askStation(train, "Destination: ");
        if (destination == NULL)
            return 0;

        if (source == destination)
            printf("Invalid source/destination: they cannot be the same station.\n");
        else if (getStationPosition(train, source->name) > getStationPosition(train, destination->name))
            printf("Invalid source/destination: train %d runs %s -> %s, "
                   "so you cannot travel from %s back to %s.\n",
                   train->trainNumber, train->source, train->destination,
                   source->name, destination->name);
        else
            break;
    }

    strcpy(passenger->source, source->name);
    strcpy(passenger->destination, destination->name);
    passenger->fare = calculateFare(train, source->name, destination->name);
    return 1;
}

/* Train is full: offers the waiting list and adds the passenger to the queue. */
void joinWaitingList(struct TrainNode *train, struct Passenger *passenger)
{
    int choice;

    printf("\nAll seats are currently booked.\n\n");
    printf("Would you like to join the waiting list?\n\n1. Yes\n2. No\n\nEnter choice: ");
    if (!readInt(&choice) || choice != 1)
    {
        printf("Booking not done.\n");
        free(passenger);
        return;
    }

    passenger->pnr = nextPnr;
    nextPnr++;
    passenger->coachNumber = 0;             /* no seat yet */
    passenger->seatNumber = 0;
    enqueue(train, passenger);

    printHeader("ADDED TO WAITING LIST");
    printf("\nPNR Number : %d\n", passenger->pnr);
    printf("Passenger  : %s\n", passenger->name);
    printf("Train      : %d - %s\n", train->trainNumber, train->trainName);
    printf("From       : %s\n", passenger->source);
    printf("To         : %s\n", passenger->destination);
    printf("Status     : WL %d\n", countWaiting(train));
    printf("Fare       : Rs. %.0f\n\n", passenger->fare);
    printf("You will be confirmed automatically when a seat is cancelled.\n");
    printLine();
}

/* Passenger: books a ticket. */
void bookTicket(void)
{
    struct TrainNode *train;
    struct Passenger *passenger;
    int coach, seat;

    printHeader("BOOK TICKET");

    /* Step 1: select a train (BST search) */
    train = askForTrain();
    if (train == NULL)
        return;
    printf("\nTrain Found: %d - %s\n", train->trainNumber, train->trainName);
    printf("Route: ");
    displayRouteInLine(train);
    printf("Seats available: %d / %d\n\n", countFreeSeats(train), TOTAL_SEATS);

    passenger = malloc(sizeof(struct Passenger));
    checkMemory(passenger);
    passenger->trainNumber = train->trainNumber;
    passenger->next = NULL;
    readPassengerDetails(passenger);

    /* Step 2: validate source and destination (DLL) */
    if (!askJourney(train, passenger))
    {
        printf("Booking cancelled.\n");
        free(passenger);
        return;
    }

    /* Step 3: check the coaches for a free seat (CLL + Array) */
    if (!allocateSeat(train, &coach, &seat))
    {
        joinWaitingList(train, passenger);  /* train is full -> Queue */
        return;
    }

    /* Step 4: book the seat */
    passenger->coachNumber = coach;
    passenger->seatNumber = seat;
    bookSeat(train, coach, seat);

    /* Step 5: generate PNR and store passenger (SLL) */
    passenger->pnr = nextPnr;
    nextPnr++;
    addPassenger(train, passenger);

    /* Step 6: record the operation for undo (Stack) */
    pushOperation(OP_BOOK, passenger, NULL);

    printHeader("BOOKING CONFIRMED");
    printf("\nPNR Number : %d\n", passenger->pnr);
    printf("Passenger  : %s\n", passenger->name);
    printf("Train      : %d\n", train->trainNumber);
    printf("Train Name : %s\n", train->trainName);
    printf("From       : %s\n", passenger->source);
    printf("To         : %s\n", passenger->destination);
    printf("Coach      : C%d\n", coach);
    printf("Seat       : %02d\n", seat);
    printf("Fare       : Rs. %.0f\n\n", passenger->fare);
    printLine();
}

/* Passenger: cancels a confirmed ticket and promotes the first waiting passenger. */
void cancelTicket(void)
{
    int pnr;
    struct TrainNode *train;
    struct Passenger *passenger, *promoted;

    printHeader("CANCEL TICKET");
    printf("Enter PNR Number: ");
    if (!readInt(&pnr))
        return;

    /* Step 1: find the booking */
    train = findTrainOfPassenger(trainRoot, pnr);
    if (train == NULL)
    {
        train = findTrainOfWaitingPassenger(trainRoot, pnr);
        if (train != NULL)
            printf("PNR %d is on the waiting list of train %d (WL %d).\n"
                   "Only confirmed tickets can be cancelled; it will be confirmed "
                   "automatically when a seat frees up.\n",
                   pnr, train->trainNumber, getWaitingPosition(train, pnr));
        else
            printf("Invalid PNR. No ticket found with PNR %d.\n", pnr);
        return;
    }

    printTicket(train, searchPassenger(train, pnr));
    printf("Confirm cancellation? (Y/N): ");
    if (!readYesNo())
    {
        printf("Cancellation aborted.\n");
        return;
    }

    /* Step 2: remove the passenger from the list (SLL) */
    passenger = removePassenger(train, pnr);

    /* Step 3: make the seat available (Array) */
    releaseSeat(train, passenger->coachNumber, passenger->seatNumber);
    printf("\nCancellation successful.\n\n");
    printf("Seat C%d-%02d is now available.\n", passenger->coachNumber, passenger->seatNumber);

    /* Step 4: promote the first waiting passenger (Queue) */
    promoted = promoteWaitingPassenger(train);

    /* Step 5: record the cancellation (and any promotion) for undo (Stack) */
    pushOperation(OP_CANCEL, passenger, promoted);
    free(passenger);
}

/* Passenger: shows a ticket (confirmed or waiting) using its PNR. */
void viewTicket(void)
{
    int pnr;
    struct TrainNode *train;

    printHeader("VIEW MY TICKET");
    printf("Enter PNR Number: ");
    if (!readInt(&pnr))
        return;

    train = findTrainOfPassenger(trainRoot, pnr);
    if (train != NULL)
    {
        printTicket(train, searchPassenger(train, pnr));
        return;
    }

    train = findTrainOfWaitingPassenger(trainRoot, pnr);
    if (train != NULL)
    {
        printTicket(train, searchWaitingList(train, pnr));
        return;
    }

    printf("Invalid PNR. No ticket found with PNR %d.\n", pnr);
}

/* Passenger: shows the waiting queue of a train. */
void viewWaitingList(void)
{
    struct TrainNode *train;
    struct Passenger *front;

    printHeader("WAITING LIST");
    train = askForTrain();
    if (train == NULL)
        return;

    printf("\n%d - %s\n\n", train->trainNumber, train->trainName);
    displayWaitingList(train);

    front = peekQueue(train);
    if (front != NULL)
        printf("\nNext to be confirmed (peek): %s, PNR %d\n", front->name, front->pnr);
}

/* Undo helper: moves a promoted passenger out of the SLL and back to the FRONT of the queue. */
void sendBackToWaitingList(struct TrainNode *train, int pnr)
{
    struct Passenger *passenger = removePassenger(train, pnr);

    if (passenger == NULL)
        return;

    releaseSeat(train, passenger->coachNumber, passenger->seatNumber);
    passenger->coachNumber = 0;
    passenger->seatNumber = 0;
    addToFrontOfQueue(train, passenger);
    printf("%s (PNR %d) moved back to the front of the waiting list.\n",
           passenger->name, passenger->pnr);
}

/* Undo a BOOK: remove the passenger and free the seat. */
void undoBooking(struct TrainNode *train, struct UndoNode *record)
{
    struct Passenger *passenger = removePassenger(train, record->ticket.pnr);

    if (passenger == NULL)
    {
        printf("PNR %d is no longer active. Nothing to undo; record discarded.\n",
               record->ticket.pnr);
        return;
    }

    releaseSeat(train, passenger->coachNumber, passenger->seatNumber);
    printf("\nUndo successful!\n\n");
    printf("Booking of %s (PNR %d) has been removed.\n", passenger->name, passenger->pnr);
    printf("Seat C%d-%02d has been released.\n", passenger->coachNumber, passenger->seatNumber);
    free(passenger);

    /* a seat became free, so give it to the first waiting passenger */
    if (!isQueueEmpty(train))
        promoteWaitingPassenger(train);
}

/* Undo a CANCEL: send any promoted passenger back to the queue, then restore the ticket. */
void undoCancellation(struct TrainNode *train, struct UndoNode *record)
{
    struct Passenger *passenger;
    int coach = record->ticket.coachNumber;
    int seat = record->ticket.seatNumber;

    if (searchPassenger(train, record->ticket.pnr) != NULL)
    {
        printf("PNR %d is already active. Nothing to undo; record discarded.\n",
               record->ticket.pnr);
        return;
    }

    if (record->hasPromotion)
        sendBackToWaitingList(train, record->promoted.pnr);

    /* use the old seat if it is free, otherwise any free seat */
    if (!isSeatFree(train, coach, seat))
    {
        if (!allocateSeat(train, &coach, &seat))
        {
            printf("Undo failed: no seat is available to restore PNR %d.\n", record->ticket.pnr);
            return;
        }
    }

    passenger = malloc(sizeof(struct Passenger));
    checkMemory(passenger);
    *passenger = record->ticket;            /* copy the saved ticket back */
    passenger->coachNumber = coach;
    passenger->seatNumber = seat;
    passenger->next = NULL;

    bookSeat(train, coach, seat);
    addPassenger(train, passenger);

    printf("\nUndo successful!\n\n");
    printf("%s's ticket has been restored.\n", passenger->name);
    printf("Seat C%d-%02d has been restored.\n", coach, seat);
}

/* Passenger: undoes the most recent booking or cancellation (top of the stack). */
void undoOperation(void)
{
    struct UndoNode *last;
    struct TrainNode *train;

    printHeader("UNDO OPERATION");
    if (isStackEmpty())
    {
        printf("Undo stack is empty. Nothing to undo.\n");
        return;
    }

    printf("Operation history:\n\n");
    displayStack();

    last = peekOperation();
    printf("\nLast Operation:\n");
    if (last->operation == OP_BOOK)
        printf("BOOK TICKET\n\n");
    else
        printf("CANCEL TICKET\n\n");
    printf("PNR       : %d\n", last->ticket.pnr);
    printf("Passenger : %s\n", last->ticket.name);
    printf("Train     : %d\n", last->ticket.trainNumber);
    printf("Coach     : C%d\n", last->ticket.coachNumber);
    printf("Seat      : %02d\n", last->ticket.seatNumber);
    if (last->hasPromotion)
        printf("Promoted  : %s (PNR %d) from waiting list\n",
               last->promoted.name, last->promoted.pnr);

    if (last->operation == OP_BOOK)
        printf("\nUndo booking? (Y/N): ");
    else
        printf("\nUndo cancellation? (Y/N): ");
    if (!readYesNo())
    {
        printf("Undo cancelled. The operation stays on the stack.\n");
        return;
    }

    last = popOperation();
    train = searchTrain(trainRoot, last->ticket.trainNumber);

    if (train == NULL)
        printf("Train %d no longer exists. Operation cannot be undone; record discarded.\n",
               last->ticket.trainNumber);
    else if (last->operation == OP_BOOK)
        undoBooking(train, last);
    else
        undoCancellation(train, last);

    free(last);
}

/* ============================ START-UP ============================ */

/* Loads the three trains that exist when the program starts. */
void loadInitialData(void)
{
    struct TrainNode *train;

    train = createTrain(12951, "Mumbai Rajdhani", "Mumbai", "Delhi");
    addStationBeforeDestination(train, "Surat");
    addStationBeforeDestination(train, "Vadodara");
    addStationBeforeDestination(train, "Ahmedabad");
    addStationBeforeDestination(train, "Kota");
    trainRoot = insertTrain(trainRoot, train);

    train = createTrain(12952, "Delhi Rajdhani", "Delhi", "Mumbai");
    addStationBeforeDestination(train, "Kota");
    addStationBeforeDestination(train, "Ahmedabad");
    addStationBeforeDestination(train, "Vadodara");
    addStationBeforeDestination(train, "Surat");
    trainRoot = insertTrain(trainRoot, train);

    train = createTrain(12431, "Rajdhani Express", "Delhi", "Mumbai");
    addStationBeforeDestination(train, "Mathura");
    addStationBeforeDestination(train, "Kota");
    addStationBeforeDestination(train, "Ratlam");
    addStationBeforeDestination(train, "Vadodara");
    addStationBeforeDestination(train, "Surat");
    trainRoot = insertTrain(trainRoot, train);
}
