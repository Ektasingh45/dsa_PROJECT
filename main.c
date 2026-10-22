/*
 * main.c
 * ------
 * Program entry point and the three menus:
 *   main menu -> Admin menu / Passenger menu
 */
#include "railway.h"

/* Admin menu: manages trains and routes. */
void adminMenu(void)
{
    int choice;

    while (1)
    {
        printHeader("ADMIN MENU");
        printf("\n1. Add Train\n");
        printf("2. Delete Train\n");
        printf("3. Search Train\n");
        printf("4. Display All Trains\n");
        printf("5. Add Station to Route\n");
        printf("6. Delete Station from Route\n");
        printf("7. Display Train Route\n");
        printf("8. Display Seat Status\n");
        printf("9. Logout\n\n");
        printf("Enter choice: ");
        if (!readInt(&choice))
            continue;

        switch (choice)
        {
        case 1: addTrain();       break;
        case 2: removeTrain();    break;
        case 3: showTrain();      break;
        case 4: showAllTrains();  break;
        case 5: addStation();     break;
        case 6: removeStation();  break;
        case 7: showRouteMenu();  break;
        case 8: showSeatStatus(); break;
        case 9:
            printf("Admin logged out.\n");
            return;
        default:
            printf("Invalid choice. Please select 1-9.\n");
        }
    }
}

/* Passenger menu: search, book, cancel, tickets, waiting list and undo. */
void passengerMenu(void)
{
    char name[NAME_LEN];
    int choice;

    printHeader("PASSENGER LOGIN");
    printf("Enter your name: ");
    readText(name);
    printf("\nWelcome, %s!\n", name);

    while (1)
    {
        printHeader("PASSENGER MENU");
        printf("\n1. Search Train\n");
        printf("2. View Train Route\n");
        printf("3. Check Seat Availability\n");
        printf("4. Book Ticket\n");
        printf("5. Cancel Ticket\n");
        printf("6. View My Ticket\n");
        printf("7. View Waiting List\n");
        printf("8. Undo Last Operation\n");
        printf("9. Display All Trains\n");
        printf("10. Logout\n\n");
        printf("Enter choice: ");
        if (!readInt(&choice))
            continue;

        switch (choice)
        {
        case 1: showTrain();       break;
        case 2: viewRoute();       break;
        case 3: showSeatStatus();  break;
        case 4: bookTicket();      break;
        case 5: cancelTicket();    break;
        case 6: viewTicket();      break;
        case 7: viewWaitingList(); break;
        case 8: undoOperation();   break;
        case 9: showAllTrains();   break;
        case 10:
            printf("Goodbye, %s! Logged out.\n", name);
            return;
        default:
            printf("Invalid choice. Please select 1-10.\n");
        }
    }
}

int main(void)
{
    int choice;

    loadInitialData();

    while (1)
    {
        printHeader("RAILWAY RESERVATION SYSTEM");
        printf("\n1. Admin Login\n");
        printf("2. Passenger Login\n");
        printf("3. Exit\n\n");
        printf("Enter choice: ");
        if (!readInt(&choice))
            continue;

        switch (choice)
        {
        case 1:
            if (adminLogin())
                adminMenu();
            break;
        case 2:
            passengerMenu();
            break;
        case 3:
            printf("\nThank you for using the Railway Reservation System!\n");
            freeAllData();
            return 0;
        default:
            printf("Invalid choice. Please select 1-3.\n");
        }
    }
}
