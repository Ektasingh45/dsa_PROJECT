================================================================================
            RAILWAY RESERVATION AND MANAGEMENT SYSTEM  (C, DSA Project)
================================================================================

Contents
  1. How to compile and run
  2. Files
  3. The seven data structures and how they work together
  4. Booking, cancellation, waiting list and undo - step by step
  5. Sample test cases
  6. Important functions for the viva
  7. Viva questions and answers
  8. Time and space complexity
  9. Possible future improvements


--------------------------------------------------------------------------------
1. HOW TO COMPILE AND RUN
--------------------------------------------------------------------------------

    gcc main.c railway.c train_bst.c route_dll.c passenger_sll.c coach_cll.c seat_array.c waiting_queue.c undo_stack.c -o railway

    Linux / macOS :  ./railway
    Windows cmd   :  railway.exe
    PowerShell    :  .\railway.exe

Admin login     : username  admin   password  1234
Passenger login : type any name

Automatic demo of the complete scenario (fills all 50 seats, waiting list,
cancellation, promotion, undo, error cases):

    Linux/macOS/cmd :  railway < demo_input.txt
    PowerShell      :  Get-Content demo_input.txt | .\railway.exe

Only stdio.h, stdlib.h and string.h are used.


--------------------------------------------------------------------------------
2. FILES
--------------------------------------------------------------------------------

railway.h        The only header. All constants (#define), all structures,
                 the global variables and every function declaration.

main.c           main(), the main menu, the Admin menu and the Passenger menu.

railway.c        All operations: admin login, add/delete train, add/delete
                 station, booking, cancellation, waiting-list promotion, undo,
                 input helpers (scanf) and loading the 3 starting trains.

train_bst.c      Binary Search Tree of trains.
route_dll.c      Doubly Linked List of stations (route).
coach_cll.c      Circular Linked List of coaches.
seat_array.c     2D Array of seats.
passenger_sll.c  Singly Linked List of confirmed passengers.
waiting_queue.c  Queue of waiting passengers.
undo_stack.c     Stack of operations for undo.

demo_input.txt   Input for the automatic demo.


--------------------------------------------------------------------------------
3. THE SEVEN DATA STRUCTURES AND HOW THEY WORK TOGETHER
--------------------------------------------------------------------------------

All trains are stored in a BST. Every train node owns its own route,
coaches, seats, passenger list and waiting queue. The undo stack is shared
by the whole system.

  BST (all trains, key = train number)
   |
   +-- Train 12951
         |-- Route       -> Doubly Linked List   Mumbai <-> Surat <-> ... <-> Delhi
         |-- Coaches     -> Circular Linked List C1 -> C2 -> ... -> C5 -> back to C1
         |-- Seats       -> Array                int seats[5][10]
         |-- Confirmed   -> Singly Linked List   [100001] -> [100002] -> NULL
         +-- Waiting     -> Queue                FRONT [Priya] -> [Arjun] REAR

  Undo Stack (whole system)                      TOP [CANCEL 100001] -> [BOOK 100050] -> ...

1) BST - finds a train by its number in O(log n) on average. Inorder
   traversal prints the trains sorted: 12431, 12627, 12951, 12952.

2) Doubly Linked List - the route. 'next' goes from source to destination,
   'prev' goes back, so the route can be printed forward and reverse.
   The position of a station checks the direction of travel and gives
   the fare: Rs. 250 + Rs. 240 x (stations travelled).

3) Circular Linked List - the coaches. The last coach points to the first.
   The train remembers 'currentCoach'; each booking starts looking there and
   then moves to the next coach, so bookings go C1, C2, C3, C4, C5, C1, ...

4) Array - seats[coach-1][seat-1] is 0 (free) or 1 (booked). Direct access.

5) Singly Linked List - confirmed passengers. New bookings are added at the end.

6) Queue - waiting passengers, First In First Out. The first person who joined
   is the first person to get a seat.

7) Stack - every booking and cancellation is pushed. Undo pops the newest one
   first (Last In First Out).

One simple idea used in the code: the same 'struct Passenger' is used for
confirmed and for waiting passengers. A waiting passenger has coachNumber 0.
When a seat frees up, the SAME node is taken out of the queue, given a seat,
and added to the confirmed list - nothing has to be copied.


--------------------------------------------------------------------------------
4. STEP BY STEP
--------------------------------------------------------------------------------

BOOKING  (bookTicket)
  1. Ask train number             -> searchTrain()          BST
  2. Read name, age, gender       -> readPassengerDetails()
  3. Read source and destination  -> askJourney()           DLL
     (must be on the route, different, and in the train's direction)
  4. Find a free seat             -> allocateSeat()         CLL + Array
     no seat -> joinWaitingList() -> enqueue()             Queue
  5. Book the seat                -> bookSeat()             Array
  6. New PNR, add passenger       -> addPassenger()         SLL
  7. Save for undo                -> pushOperation(OP_BOOK) Stack
  8. Print the ticket

CANCELLATION  (cancelTicket)
  1. Find the PNR                 -> findTrainOfPassenger() BST + SLL
  2. Show ticket, ask Y/N
  3. Remove passenger             -> removePassenger()      SLL
  4. Free the seat                -> releaseSeat()          Array
  5. Promote first waiting person -> promoteWaitingPassenger()
                                     dequeue() -> allocateSeat() -> addPassenger()
  6. Save for undo                -> pushOperation(OP_CANCEL, ticket, promoted)
  7. Free the cancelled node

WAITING LIST
  Train full -> passenger agrees -> gets a PNR -> enqueue() at the REAR.
  When a seat frees (cancel, or undo of a booking) the FRONT passenger is
  confirmed automatically with the same PNR.

UNDO  (undoOperation)
  Shows the stack, shows the top operation and asks Y/N, then pops it.
  - Train was deleted?  -> record is discarded.
  - Undo BOOK           -> remove the passenger, free the seat, promote a
                           waiting passenger if there is one.
  - Undo CANCEL         -> if someone was promoted, move them back to the
                           FRONT of the queue (sendBackToWaitingList), then
                           re-create the cancelled ticket from the copy saved
                           in the stack, in the same seat.
  The stack saves a COPY of the ticket (not a pointer) because the cancelled
  passenger node has already been freed.


--------------------------------------------------------------------------------
5. SAMPLE TEST CASES
--------------------------------------------------------------------------------

Test 1 - Booking and coach rotation
  Passenger > 4, train 12951, Rahul, 20, 1, Mumbai, Delhi
  Expected: PNR 100001, C1 seat 01, Rs. 1450.
  Book again on 12951 -> C2 seat 01 (circular list moved to the next coach).

Test 2 - Invalid journey
  Passenger > 4, train 12951, source Delhi, destination Mumbai
  Expected: "you cannot travel from Delhi back to Mumbai", asked again.
  Source Pune -> "Invalid station". Enter 0 -> "Booking cancelled."

Test 3 - Waiting list
  Book 50 tickets on one train (or run demo_input.txt).
  51st booking -> "All seats are currently booked" -> 1 -> "Status : WL 1".
  Passenger > 7 shows the queue FRONT ... REAR.

Test 4 - Cancellation with promotion
  Passenger > 5, PNR 100001, Y
  Expected: "Seat C1-01 is now available." then
            "Priya (PNR 100051) has been automatically confirmed. Allotted seat: C1-01"

Test 5 - Undo cancellation
  Passenger > 8, Y
  Expected: "Priya (PNR 100051) moved back to the front of the waiting list."
            "Rahul's ticket has been restored. Seat C1-01 has been restored."

Test 6 - Undo after the train is deleted
  Book on 12951, Admin deletes 12951, Passenger > 8, Y
  Expected: "Train 12951 no longer exists ... record discarded." (no crash)

Test 7 - Admin route
  Admin > 5, 12627, Agra, 0  -> station added before the destination.
  Admin > 5, 12627, Agra     -> "Duplicate station."
  Admin > 6, 12627, Pune     -> "Station not found."
  Admin > 6, 12627, New Delhi-> cannot delete source/destination.


--------------------------------------------------------------------------------
6. IMPORTANT FUNCTIONS FOR THE VIVA
--------------------------------------------------------------------------------

BST          createTrain, insertTrain, searchTrain, deleteTrain, displayAllTrains
DLL          addStationAtEnd, insertStationAfter, deleteStation,
             displayRouteForward, displayRouteBackward, getStationPosition
CLL          createCoaches, getNextCoach, displayCoaches, freeCoaches
Array        initSeats, findFreeSeat, bookSeat, releaseSeat, displaySeats
SLL          addPassenger, searchPassenger, removePassenger, displayPassengers
Queue        enqueue, dequeue, peekQueue, addToFrontOfQueue, displayWaitingList
Stack        pushOperation, popOperation, peekOperation, displayStack
Main logic   bookTicket, allocateSeat, cancelTicket, promoteWaitingPassenger,
             undoOperation, undoBooking, undoCancellation, addTrain, addStation


--------------------------------------------------------------------------------
7. VIVA QUESTIONS AND ANSWERS
--------------------------------------------------------------------------------

Q1. Why a BST for trains?
A.  Trains are searched by number all the time and are added/deleted at run
    time. A BST searches in O(log n) on average and inorder traversal gives a
    sorted list for free.

Q2. Worst case of the BST?
A.  O(n), when train numbers are inserted in sorted order - the tree becomes
    a straight line. AVL / Red-Black trees fix this.

Q3. How does deleteTrain handle a node with two children?
A.  It finds the inorder successor (smallest train in the right subtree),
    frees the deleted train's lists, copies the successor's train data into
    the node (keeping the node's left/right links), and removes the old
    successor node from the right subtree.

Q4. Why is the route a doubly linked list?
A.  We need forward AND reverse printing, and stations are inserted in the
    middle. 'prev' makes reverse traversal and unlinking easy.

Q5. How is source/destination validated?
A.  Both stations must be found in the route; they must be different; and
    the source's position must be smaller than the destination's.

Q6. What does the circular linked list do?
A.  It holds the coaches. currentCoach moves to ->next after each booking;
    after C5 comes C1, so the allocation rotates with no index arithmetic.

Q7. How do you stop traversing a circular list?
A.  With a do-while loop that stops when we come back to coachHead.

Q8. Why an array for seats?
A.  The layout is fixed (5 x 10) and seats are accessed by position, so
    booking/checking a seat is O(1).

Q9. Why a queue for the waiting list?
A.  Waiting lists are first come, first served (FIFO). With front and rear
    pointers both enqueue and dequeue are O(1).

Q10. addToFrontOfQueue adds at the front - doesn't that break FIFO?
A.  It is used only by undo, to put a promoted passenger back exactly where
    they were. New passengers always join at the rear with enqueue().

Q11. Why does the stack store a copy of the ticket?
A.  Cancelling frees the passenger node. A pointer would point to freed
    memory; a copy can always be used to restore the ticket.

Q12. Why is one 'struct Passenger' used for both the list and the queue?
A.  Both store the same ticket data. A promoted passenger node is simply
    moved from the queue into the list - no copying and no new malloc.

Q13. How is the PNR unique?
A.  One counter, nextPnr, starts at 100001 and increases for every ticket.

Q14. How is memory freed?
A.  Every malloc is checked by checkMemory(). Deleting a train frees its
    route, coaches, passengers and waiting list (freeTrainData). On exit,
    freeAllData() frees the whole BST and the stack. Tested with a
    malloc/free counter: 0 allocations left at exit.

Q15. What happens on wrong input like letters instead of a number?
A.  readInt() checks scanf's return value; if it is not 1 it prints
    "Invalid input", clears the line and the menu is shown again.

Q16. Why can't the admin delete some stations?
A.  The source and destination define the train, and a station used by a
    booked or waiting passenger would make their ticket invalid.


--------------------------------------------------------------------------------
8. TIME AND SPACE COMPLEXITY
--------------------------------------------------------------------------------

  n = trains, h = BST height, k = stations, p = confirmed, w = waiting,
  P = all passengers in the system

  BST insert / search / delete     O(h)  (average O(log n), worst O(n))
  BST inorder display              O(n)
  DLL search / insert / delete     O(k)
  CLL next coach                   O(1)
  Array book / release / check     O(1)
  Find free seat (allocateSeat)    O(5 x 10) = constant
  SLL add / search / remove        O(p)
  Queue enqueue / dequeue / peek   O(1)
  Stack push / pop / peek          O(1)
  Book ticket                      O(h + k + p)
  Cancel ticket                    O(n + P)   (PNR is searched in every train)

  Space: each train uses O(k + 5 + 50 + p + w); the whole system uses
  O(n x (k + 55) + P + undo records).


--------------------------------------------------------------------------------
9. POSSIBLE FUTURE IMPROVEMENTS
--------------------------------------------------------------------------------

  - Save and load data from files.
  - AVL tree so the BST always stays balanced.
  - Journey dates, coach classes and berth preferences.
  - Cancelling a waiting-list ticket.
  - Searching trains between two stations.
================================================================================
