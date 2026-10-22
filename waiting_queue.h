/*
 * waiting_queue.h
 * ---------------
 * QUEUE module - waiting list of one train (linked implementation, FIFO).
 *
 *   FRONT                    REAR
 *     |                        |
 *     v                        v
 *   [Priya] -> [Arjun] -> [Meera] -> NULL
 *
 * Enqueue at REAR, dequeue from FRONT, both O(1).
 */
#ifndef WAITING_QUEUE_H
#define WAITING_QUEUE_H

#ifndef NAME_LEN
#define NAME_LEN 100
#endif
#ifndef GENDER_LEN
#define GENDER_LEN 20
#endif

typedef struct WaitingPassenger
{
    int pnr;
    char name[NAME_LEN];
    int age;
    char gender[GENDER_LEN];
    char source[NAME_LEN];
    char destination[NAME_LEN];
    float fare;
    struct WaitingPassenger *next;
} WaitingPassenger;

typedef struct WaitingQueue
{
    WaitingPassenger *front;
    WaitingPassenger *rear;
    int size;
} WaitingQueue;

void initQueue(WaitingQueue *q);
int isQueueEmpty(const WaitingQueue *q);

/* Adds at REAR. 1 = success, 0 = memory failure. */
int enqueue(WaitingQueue *q, int pnr, const char *name, int age, const char *gender,
            const char *source, const char *destination, float fare);

/* Removes from FRONT, copies the data into *out and frees the node. 1 = success, 0 = empty. */
int dequeue(WaitingQueue *q, WaitingPassenger *out);

WaitingPassenger *peekQueue(const WaitingQueue *q);

/*
 * Undo support only: puts a passenger back at the FRONT, i.e. reverses a
 * dequeue. Normal joining always uses enqueue(), so FIFO order is preserved.
 */
int restoreFront(WaitingQueue *q, const WaitingPassenger *data);

/* 1-based position of the PNR in the queue, 0 if absent (read-only scan). */
int queuePosition(const WaitingQueue *q, int pnr);
WaitingPassenger *searchQueue(const WaitingQueue *q, int pnr);

int queueUsesStation(const WaitingQueue *q, const char *station);

void displayQueue(const WaitingQueue *q);
void freeQueue(WaitingQueue *q);

#endif
