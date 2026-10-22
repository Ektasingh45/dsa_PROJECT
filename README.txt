================================================================================
            RAILWAY RESERVATION AND MANAGEMENT SYSTEM  (C, DSA Project)
================================================================================

Contents
  0.  How to compile and run
  1.  Project overview
  2.  File-by-file explanation
  3.  The seven data structures
  4.  Why each data structure was selected
  5.  Complete data flow
  6.  Booking flow
  7.  Cancellation flow
  8.  Waiting-list flow
  9.  Undo flow
  10. Time complexity of major operations
  11. Space complexity
  12. Sample output
  13. Viva questions and answers
  14. Possible future improvements


--------------------------------------------------------------------------------
0. HOW TO COMPILE AND RUN
--------------------------------------------------------------------------------

Compile (Linux / macOS / Windows with MinGW or MSYS2):

    gcc main.c railway.c train_bst.c route_dll.c passenger_sll.c coach_cll.c seat_array.c waiting_queue.c undo_stack.c -o railway

Run on Linux / macOS:

    ./railway

Run on Windows: GCC creates railway.exe. In Command Prompt type

    railway.exe

and in PowerShell type

    .\railway.exe

Optional strict build (the project compiles with no warnings):

    gcc -Wall -Wextra -pedantic -std=c99 main.c railway.c train_bst.c route_dll.c passenger_sll.c coach_cll.c seat_array.c waiting_queue.c undo_stack.c -o railway

Login details
    Admin     : username  admin    password  1234
    Passenger : enter any name (no password)

Automatic demo (runs the complete 20-step scenario, including filling all
50 seats of a new train, waiting list, cancellation, promotion and undo):

    Linux/macOS/cmd :  railway < demo_input.txt
    PowerShell      :  Get-Content demo_input.txt | .\railway.exe

Only standard headers are used: stdio.h, stdlib.h, string.h.


--------------------------------------------------------------------------------
1. PROJECT OVERVIEW
--------------------------------------------------------------------------------

The program is a menu-driven railway reservation system with two roles:

  ADMIN      manages infrastructure: add/delete/search trains, add/delete
             stations on a route, view routes and the full seat status
             (seat map, confirmed passengers, waiting list).

  PASSENGER  uses the railway: search trains, view routes, check seats,
             book, cancel, view a ticket by PNR, view a waiting list,
             undo the last booking/cancellation, list all trains.
             Passengers never see any infrastructure option.

Three trains are preloaded by loadInitialData():

  12951  Mumbai Rajdhani   Mumbai <-> Surat <-> Vadodara <-> Ahmedabad <-> Kota <-> Delhi
  12952  Delhi Rajdhani    Delhi <-> Kota <-> Ahmedabad <-> Vadodara <-> Surat <-> Mumbai
  12431  Rajdhani Express  Delhi <-> Mathura <-> Kota <-> Ratlam <-> Vadodara <-> Surat <-> Mumbai

Every train (preloaded or added by the admin) owns its own:

  Train (BST node)
   |-- Route                -> Doubly Linked List
   |-- Coaches              -> Circular Linked List (5 coaches)
   |-- Seats                -> 2D Array int seats[5][10]
   |-- Confirmed passengers -> Singly Linked List
   +-- Waiting list         -> Queue

The system as a whole owns:
  - the BST of all trains,
  - one undo Stack of booking/cancellation operations,
  - the PNR counter (starts at 100001, every ticket gets a unique PNR).

Fare rule: Rs. 250 base + Rs. 240 for each route segment travelled.
Mumbai -> Delhi on 12951 is 5 segments, so the fare is 250 + 5 x 240 = Rs. 1450.


--------------------------------------------------------------------------------
2. FILE-BY-FILE EXPLANATION
--------------------------------------------------------------------------------

main.c
    Entry point. Calls loadInitialData(), runRailwaySystem() and
    shutdownRailwaySystem(). Contains no logic of its own.

railway.h / railway.c   (central controller)
    Owns the global state: BST root, undo stack, PNR counter.
    Contains the main menu, admin login, admin menu, passenger menu, all
    input validation, ticket printing, fare calculation, and the
    workflows that combine several data structures: booking,
    cancellation, waiting-list promotion and undo. This is the only file
    that "knows" all seven structures; every other module only knows itself.

train_bst.h / train_bst.c   (Binary Search Tree)
    Defines struct TrainNode (train data + pointers to that train's own
    route, coaches, seats, passengers and queue + left/right children).
    createTrain() builds a train together with all its sub-structures.
    Implements insert, search, delete (all three cases), inorder
    traversal, count, and freeing a train / the whole tree.

route_dll.h / route_dll.c   (Doubly Linked List)
    struct Station {name, prev, next} and struct Route {head, tail, count}.
    Add at end, add after a station, add just before the destination,
    delete, search, position (index) of a station, display forward,
    display backward, display inline, free. Station comparison is
    case-insensitive (namesEqual).

coach_cll.h / coach_cll.c   (Circular Linked List)
    struct Coach {coachNumber, next}. Creates C1..C5 with the last node
    pointing back to C1, displays the circle, gets the next coach,
    counts coaches and frees the circle.

seat_array.h / seat_array.c   (Array)
    Works on int seats[5][10] (0 = available, 1 = booked). Initialise,
    display one coach / all coaches, find free seat in a coach, book,
    release, check availability, count free seats.

passenger_sll.h / passenger_sll.c   (Singly Linked List)
    struct Passenger exactly as specified (pnr, name, age, gender, train,
    coach, seat, source, destination, fare, next). Create node, add at
    tail, search by PNR, remove (unlink and return), delete (unlink and
    free), display all, count, check if a station is in use, free list.

waiting_queue.h / waiting_queue.c   (Queue)
    Linked FIFO queue with front and rear pointers. Enqueue, dequeue,
    peek, is-empty, display (FRONT ... REAR), position of a PNR,
    restoreFront (used ONLY by undo to reverse a dequeue), free.

undo_stack.h / undo_stack.c   (Stack)
    Linked LIFO stack of Operation records (BOOK / CANCEL + full ticket
    snapshot + optional snapshot of a promoted waiting passenger).
    Push, pop, peek, is-empty, display, free.

demo_input.txt
    Scripted input for the complete sample scenario (see section 0).

README.txt
    This documentation.


--------------------------------------------------------------------------------
3. THE SEVEN DATA STRUCTURES
--------------------------------------------------------------------------------

1) ARRAY - seat map of a train
       int seats[5][10];       seats[coach-1][seat-1]
       0 = Available (A), 1 = Booked (X)

       Coach C1:
       01[A] 02[A] 03[X] 04[A] 05[A]
       06[A] 07[X] 08[A] 09[A] 10[A]

2) SINGLY LINKED LIST - confirmed passengers of a train
       head -> [100001 Rahul] -> [100002 Amit] -> [100003 Neha] -> NULL
   New bookings are appended at the tail, so the list is in booking order.

3) DOUBLY LINKED LIST - route of a train
       NULL <- Mumbai <-> Surat <-> Vadodara <-> Ahmedabad <-> Kota <-> Delhi -> NULL
       head = source, tail = destination
   Forward traversal follows next, reverse traversal follows prev.

4) CIRCULAR LINKED LIST - coaches of a train
       C1 -> C2 -> C3 -> C4 -> C5
       ^                       |
       +-----------------------+
   The train keeps a rotation pointer (nextCoach). Each booking starts
   looking for a seat at that coach and then moves the pointer one step,
   so bookings go C1, C2, C3, C4, C5, C1, ... (round-robin).

5) QUEUE - waiting list of a train (FIFO, linked)
       FRONT -> [Priya] -> [Arjun] -> [Meera] <- REAR
   Joining = enqueue at rear. Promotion = dequeue from front.

6) STACK - undo history (LIFO, linked, system-wide)
       TOP -> [CANCEL 100005] -> [BOOK 100004] -> [CANCEL 100003] -> NULL
   Every successful booking/cancellation is pushed. Undo pops the top.

7) BINARY SEARCH TREE - all trains, key = train number
   After loading 12951, 12952, 12431 and the admin adding 12627:

                 12951
                /     \
            12431     12952
                \
               12627

   Inorder traversal gives sorted order:
       12431, 12627, 12951, 12952


--------------------------------------------------------------------------------
4. WHY EACH DATA STRUCTURE WAS SELECTED
--------------------------------------------------------------------------------

BST (trains)
    Trains are looked up by number in almost every operation. A BST gives
    O(log n) average search/insert/delete without needing a fixed size,
    and inorder traversal prints the timetable sorted by number for free.

Doubly Linked List (route)
    Routes change (admin inserts/removes stations anywhere in the middle)
    and must be shown in both directions (onward and return journey).
    prev/next links give O(1) insertion/deletion at a known node and
    natural reverse traversal without recursion or an extra array.
    The position of a station in the list also validates the direction
    of travel and gives the number of segments used for the fare.

Circular Linked List (coaches)
    Seat allocation should rotate through coaches endlessly. In a circular
    list "next of last = first", so rotation is just cur = cur->next with
    no index arithmetic or end-of-list checks.

Array (seats)
    The seat layout is fixed (5 x 10) and seats are accessed by position
    (coach, seat). An array gives O(1) direct access to book, release or
    check any seat, with no pointer overhead.

Singly Linked List (confirmed passengers)
    The number of passengers changes constantly and cancellations remove
    passengers from anywhere in the list. A linked list grows and shrinks
    with no shifting of elements; only forward traversal is needed.

Queue (waiting list)
    Railway waiting lists are first-come-first-served. A queue enforces
    FIFO; with front and rear pointers both enqueue and dequeue are O(1).

Stack (undo)
    Undo must reverse the MOST RECENT operation first. That is exactly
    LIFO. Push and pop are O(1).


--------------------------------------------------------------------------------
5. COMPLETE DATA FLOW
--------------------------------------------------------------------------------

                         main.c
                           |
                    loadInitialData()  -> builds 3 trains (BST + all sub-structures)
                           |
                   runRailwaySystem()  (railway.c)
                           |
             +-------------+--------------+
             |                            |
          ADMIN                       PASSENGER
   add/delete train  -> BST       search train   -> BST
   add/delete station-> DLL       view route     -> BST + DLL (fwd/rev)
   display route     -> DLL       seat status    -> BST + CLL + Array
   seat status       -> CLL,      book ticket    -> BST, DLL, Array, CLL, SLL, Stack (Queue if full)
                        Array,    cancel ticket  -> SLL, Array, Queue, SLL, Stack
                        SLL,      view ticket    -> BST walk + SLL / Queue
                        Queue     waiting list   -> Queue
                                  undo           -> Stack, BST, SLL, Array, Queue
                           |
               shutdownRailwaySystem()  -> frees every node of every structure


--------------------------------------------------------------------------------
6. BOOKING FLOW
--------------------------------------------------------------------------------

  Passenger enters train number
        |
  BST search (searchTrain)              -> "Train not found." if absent
        |
  Read name, age (1-120), gender (menu)
        |
  Read source and destination
  Validate with route DLL (searchStation, stationIndex):
    - both must be on the route
    - must be different
    - source must come before destination (direction of travel)
        |
  Fare = 250 + 240 x (index(dest) - index(source))
        |
  Seat Array: countAvailableSeats == 0 ?
        |-- yes -> "All seats are currently booked. Join waiting list?"
        |          Yes -> new PNR, ENQUEUE into the train's queue, show WL number
        |
        +-- no  -> allocateSeat():
                   start at train->nextCoach (circular list)
                   findAvailableSeatInCoach() in the Array
                   if coach full -> getNextCoach() and try again
                   move nextCoach one step forward
        |
  Generate PNR (nextPnr++)
        |
  createPassenger + addPassenger        -> Singly Linked List
        |
  bookSeat                              -> Array (seat becomes X)
        |
  push(OP_BOOK, ticket snapshot)        -> Stack
        |
  Print "BOOKING CONFIRMED"


--------------------------------------------------------------------------------
7. CANCELLATION FLOW
--------------------------------------------------------------------------------

  Passenger enters PNR
        |
  Walk the BST, search each train's SLL for the PNR
        |-- not found but in a waiting queue -> explain it is waitlisted
        |-- not found anywhere               -> "Invalid PNR."
        |
  Display ticket, ask for confirmation
        |
  Copy ticket into a TicketRecord (snapshot)
  removePassenger + free                -> Singly Linked List
        |
  releaseSeat                           -> Array   "Seat C2-01 is now available."
        |
  "Checking Waiting List..."            -> Queue
  If not empty: promote front passenger (section 8)
        |
  push(OP_CANCEL, snapshot, promoted passenger snapshot if any) -> Stack


--------------------------------------------------------------------------------
8. WAITING-LIST FLOW
--------------------------------------------------------------------------------

  Joining:
     Train full -> passenger agrees -> PNR assigned immediately
     -> enqueue at REAR -> status "WL n" (n = position in queue)

  Automatic promotion (promoteFromWaitingList):
     A seat is freed (cancellation, or undo of a booking)
        |
     Queue empty? -> "Waiting list is empty."
        |
     dequeue FRONT passenger (the one who waited longest)
        |
     allocateSeat() with the circular coach list + array
        |
     createPassenger with the SAME PNR -> addPassenger (SLL)
        |
     bookSeat (Array)
        |
     "Priya has been automatically confirmed."

  Example:
     Confirmed: Rahul (100001) ...        Waiting: Priya -> Arjun
     Rahul cancels:
        remove Rahul (SLL) -> free C1-01 (Array) -> dequeue Priya (Queue)
        -> give Priya C1-01 -> add Priya (SLL) -> push CANCEL (Stack)
     Waiting list is now: Arjun

  The waitlisted PNR can be checked with "View My Ticket", which shows
  "WAITING LIST (WL n)" until the passenger is confirmed.


--------------------------------------------------------------------------------
9. UNDO FLOW
--------------------------------------------------------------------------------

  Each stack node stores a COPY of the ticket (not a pointer), because the
  live passenger node is freed on cancellation. A pointer would dangle;
  a copy is always safe to restore from.

  Undo menu:
     Stack empty?  -> "Undo stack is empty. Nothing to undo."
     Show history (TOP first) and the last operation's details
     Ask "Undo booking/cancellation? (Y/N)"
        N -> operation stays on the stack
        Y -> pop, then BST search for the train
             train deleted by admin? -> record discarded with a message

  Undo BOOK:
     remove the PNR from the SLL, release its seat in the Array.
     If someone is waiting, the freed seat is given to the queue front.
     (If the ticket was already cancelled, the record is stale and discarded.)

  Undo CANCEL:
     1. If the cancellation had promoted a waiting passenger, remove that
        passenger from the SLL, free their seat and put them back at the
        FRONT of the queue (restoreFront) - exactly where they were.
     2. Re-create the cancelled passenger with the same PNR, in the same
        coach/seat (any free seat if that one is somehow taken).
     3. Mark the seat booked and add the passenger to the SLL.

        Undo successful!
        Rahul's ticket has been restored.
        Seat C1-01 has been restored.

  Because undo always takes the top of the stack, operations are reversed
  strictly newest-first, which keeps seats, lists and queues consistent.


--------------------------------------------------------------------------------
10. TIME COMPLEXITY OF MAJOR OPERATIONS
--------------------------------------------------------------------------------

  n = number of trains, h = BST height (log n average, n worst)
  k = stations on a route, C = coaches (5), S = seats per coach (10)
  p = confirmed passengers on a train, w = waiting passengers on a train
  P = total passengers in the whole system

  BST    insert / search / delete            O(h)  avg O(log n), worst O(n)
  BST    inorder display                     O(n)
  DLL    add station (after search)          O(k)  (link itself is O(1))
  DLL    delete / search station             O(k)
  DLL    display forward / reverse           O(k)
  CLL    get next coach                      O(1)
  CLL    display / count                     O(C)
  Array  book / release / check seat         O(1)
  Array  find free seat in a coach           O(S)
  Array  count free seats                    O(C x S)
  SLL    add passenger (at tail)             O(p)
  SLL    search / delete by PNR              O(p)
  Queue  enqueue / dequeue / peek            O(1)
  Queue  display                             O(w)
  Stack  push / pop / peek                   O(1)

  Book ticket      O(h + k + C x S + p)
  Cancel ticket    O(n + P)       (PNR is searched across all trains)
  Promotion        O(C x S + p)
  Undo             O(h + p + C x S)
  Delete train     O(h + k + C + p + w)

  With C x S fixed at 50 those terms are effectively constant.


--------------------------------------------------------------------------------
11. SPACE COMPLEXITY
--------------------------------------------------------------------------------

  Per train : O(k + C + C x S + p + w)
              route nodes + coach nodes + 50 ints + passenger nodes + queue nodes
  Whole system : O(n x (k + C x S) + P + W + U)
              W = all waiting passengers, U = operations on the undo stack
  Recursion  : BST insert/delete/inorder use O(h) stack space.

  All memory comes from malloc(); every allocation is checked for NULL.
  Deleting a train frees its route, coaches, passengers and queue.
  On exit (menu option 3, or end of input) everything is freed.
  Tested with a counting malloc/free wrapper over the full demo scenario:
  158 allocations, 0 still live at exit.


--------------------------------------------------------------------------------
12. SAMPLE OUTPUT (abridged)
--------------------------------------------------------------------------------

================================================
           RAILWAY RESERVATION SYSTEM
================================================

1. Admin Login
2. Passenger Login
3. Exit

Enter choice: 1
Username: admin
Password: 1234

Login successful. Welcome, Admin!

------ Admin > 1. Add Train ------
Enter Train Number: 12627
Enter Train Name: Karnataka Express
Enter Source: New Delhi
Enter Destination: Bangalore

Train added successfully!
Route created   : New Delhi <-> Bangalore
Coaches created : C1 -> C2 -> C3 -> C4 -> C5 -> (back to C1)
Seats created   : 50 (all available)

------ Admin > 5. Add Station to Route (repeated for Agra, Gwalior, Bhopal, Nagpur) ------
Enter Train Number: 12627
Current route: New Delhi <-> Bangalore
Enter Station Name: Agra
Insert after which station? (press Enter to add just before Bangalore):
Station added successfully.

Updated route:
New Delhi <-> Agra <-> Bangalore
...
New Delhi <-> Agra <-> Gwalior <-> Bhopal <-> Nagpur <-> Bangalore

------ Admin > 4. Display All Trains ------
12431 - Rajdhani Express       Delhi        -> Mumbai       Seats 50/50  WL 0
12627 - Karnataka Express      New Delhi    -> Bangalore    Seats 50/50  WL 0
12951 - Mumbai Rajdhani        Mumbai       -> Delhi        Seats 50/50  WL 0
12952 - Delhi Rajdhani         Delhi        -> Mumbai       Seats 50/50  WL 0

------ Passenger > 4. Book Ticket ------
Enter Train Number: 12627
Passenger Name: Rahul
Age: 20
Gender (1. Male  2. Female  3. Other)
Enter choice: 1
Source: New Delhi
Destination: Bangalore

================================================
               BOOKING CONFIRMED
================================================

PNR Number : 100001
Passenger  : Rahul
Train      : 12627
Train Name : Karnataka Express
From       : New Delhi
To         : Bangalore
Coach      : C1
Seat       : 01
Fare       : Rs. 1450

================================================

(next bookings get C2-01, C3-01, C4-01, C5-01, C1-02, ... until all 50 are taken)

------ Passenger > 4. Book Ticket (train full) ------
All seats are currently booked.

Would you like to join the waiting list?

1. Yes
2. No

Enter choice: 1

================================================
             ADDED TO WAITING LIST
================================================

PNR Number : 100051
Passenger  : Priya
Status     : WL 1

------ Passenger > 5. Cancel Ticket ------
Enter PNR Number: 100001
(ticket shown)
Confirm cancellation? (Y/N): Y

Cancellation successful.

Seat C1-01 is now available.

Checking Waiting List...

Waiting passenger found!

Priya (PNR 100051) has been automatically confirmed.
Allotted seat: C1-01

------ Passenger > 8. Undo Last Operation ------
TOP
 |
 v
CANCEL PNR 100001  (Rahul, Train 12627, C1-01)  [promoted PNR 100051]
BOOK   PNR 100050  (Passenger50, Train 12627, C5-10)
...

Last Operation:
CANCEL TICKET

PNR       : 100001
Passenger : Rahul
Train     : 12627
Coach     : C1
Seat      : 01
Promoted  : Priya (PNR 100051) from waiting list

Undo cancellation? (Y/N): Y
Priya (PNR 100051) moved back to the front of the waiting list.

Undo successful!

Rahul's ticket has been restored.
Seat C1-01 has been restored.


--------------------------------------------------------------------------------
13. VIVA QUESTIONS AND ANSWERS
--------------------------------------------------------------------------------

Q1. Why did you use a BST for trains instead of an array?
A.  Trains are added and deleted at run time and searched by number. A BST
    gives O(log n) average search/insert/delete with no fixed size and no
    shifting, and inorder traversal lists trains in sorted order.

Q2. What is the worst case of your BST and when does it happen?
A.  O(n), when keys are inserted in sorted order (e.g. 10001, 10002, ...);
    the tree degenerates into a linked list. An AVL or Red-Black tree would
    guarantee O(log n).

Q3. Explain BST deletion of a node with two children in your code.
A.  We find the inorder successor (leftmost node of the right subtree),
    detach it from its parent, and relink it into the deleted node's place
    (it takes over the left and right children). We relink instead of
    copying data because each node owns a route, coaches and lists - moving
    the node moves all of them safely. The removed node is then freed with
    all its sub-structures.

Q4. Why is the route a doubly linked list and not a singly linked list?
A.  The route must be shown in reverse (return journey) and stations are
    inserted/deleted in the middle. With prev pointers, reverse traversal
    is direct from the tail and unlinking needs no "previous" search.

Q5. How do you validate source and destination?
A.  Both must be found in the train's route DLL, must differ, and the
    index of the source must be smaller than that of the destination
    (the train only runs in one direction).

Q6. What is the use of the circular linked list?
A.  It holds the coaches. The train keeps a pointer to the coach where the
    next allocation starts; after each booking it moves to ->next. Since the
    last coach points to the first, allocation rotates C1..C5 forever
    without bounds checks, distributing passengers across coaches.

Q7. How do you detect the end of traversal in a circular list?
A.  Using a do-while loop that stops when the pointer comes back to the
    starting node (cur != head), because there is no NULL.

Q8. Why is the seat map an array?
A.  The layout is fixed (5 x 10) and seats are accessed by (coach, seat).
    seats[c-1][s-1] gives O(1) booking, releasing and checking.

Q9. Why is the waiting list a queue? Why linked and not array based?
A.  Waiting lists are first-come-first-served (FIFO). A linked queue has no
    fixed capacity and no wasted slots; front/rear pointers make enqueue
    and dequeue O(1).

Q10. Your restoreFront() adds at the front - does that break FIFO?
A.  No. It is used only by undo, to reverse a dequeue. It puts the passenger
    back exactly where they were, so the original FIFO order is restored.
    New passengers always join with enqueue() at the rear.

Q11. What exactly is stored on the undo stack and why not a pointer?
A.  The operation type, a full copy of the ticket (PNR, name, age, gender,
    train, coach, seat, stations, fare) and, for a cancellation that
    promoted someone, a copy of the promoted passenger. A pointer would
    dangle because cancelled passenger nodes are freed.

Q12. What happens if the admin deletes a train and the passenger presses undo?
A.  The record is popped, the BST search for its train fails, and the record
    is discarded with a message. Nothing points into freed memory because
    the stack holds copies.

Q13. How is the PNR kept unique?
A.  A single global counter starting at 100001, incremented for every
    confirmed or waitlisted ticket. A promoted passenger keeps the PNR they
    received when they joined the waiting list.

Q14. Walk through what happens when a confirmed passenger cancels.
A.  SLL search finds the PNR -> node is unlinked and freed -> seat released
    in the array -> queue checked; if not empty the front passenger is
    dequeued, given a seat via the circular coach list, added to the SLL
    and the seat marked booked -> a CANCEL record (with promotion info) is
    pushed on the stack.

Q15. Which data structures does one booking touch?
A.  BST (find train), DLL (validate stations, fare), Array (availability
    and seat), CLL (coach rotation), SLL (add passenger), Stack (push
    BOOK), and the Queue if the train is full.

Q16. How do you avoid memory leaks?
A.  Every malloc is checked for NULL. Each module has a free function
    (freeRoute, freeCoaches, freePassengerList, freeQueue, freeStack,
    freeTrain, freeAllTrains). Deleting a train frees all its structures;
    on exit shutdownRailwaySystem() frees everything. Verified with a
    counting malloc/free wrapper: 0 live allocations at exit.

Q17. How does the program handle invalid input?
A.  All input is read line-by-line with fgets and converted with strtol, so
    letters in a numeric field print "Invalid input" instead of looping
    forever like scanf would. Over-long lines are discarded, empty names
    are rejected, and end of input exits cleanly after freeing memory.

Q18. Why is the stack system-wide but the queue per train?
A.  Undo reverses the user's latest action regardless of train, so one
    global history is needed. Waiting lists are naturally separate for each
    train, so each train has its own queue.

Q19. What is the difference between removePassenger and deletePassenger?
A.  removePassenger only unlinks and returns the node (the caller reads it,
    e.g. to snapshot it, then frees it). deletePassenger unlinks and frees.

Q20. Why can't the admin delete a station that has bookings?
A.  Those tickets would refer to a station that no longer exists on the
    route, which would make the tickets and fare invalid. The source and
    destination termini can never be deleted for the same reason.


--------------------------------------------------------------------------------
14. POSSIBLE FUTURE IMPROVEMENTS
--------------------------------------------------------------------------------

  - Save/load trains and reservations to files so data survives restarts.
  - Self-balancing tree (AVL / Red-Black) to guarantee O(log n) search.
  - Hash table from PNR to passenger node for O(1) PNR lookup.
  - Journey dates and multiple runs of the same train.
  - Segment-wise seat reuse (a seat freed at an intermediate station can be
    resold for the remaining part of the journey).
  - Coach classes (Sleeper, 3A, 2A, 1A) with different fares and layouts.
  - Seat preferences (window / lower berth) and concessions (senior citizen).
  - Cancelling a waitlisted ticket and RAC (Reservation Against Cancellation).
  - Passenger accounts with passwords, and hashed admin password.
  - Search trains between two stations by scanning every route DLL.
================================================================================
