/*
 * coach_cll.c
 * -----------
 * Implementation of the coach CIRCULAR LINKED LIST.
 */
#include <stdio.h>
#include <stdlib.h>
#include "coach_cll.h"

Coach *createCoaches(int count)
{
    Coach *head = NULL, *tail = NULL, *node;
    int i;

    for (i = 1; i <= count; i++)
    {
        node = (Coach *)malloc(sizeof(Coach));
        if (node == NULL)
        {
            printf("Memory allocation failed while creating coaches.\n");
            if (tail != NULL)
                tail->next = head; /* close the circle so freeCoaches works */
            freeCoaches(&head);
            return NULL;
        }
        node->coachNumber = i;
        node->next = NULL;

        if (head == NULL)
            head = node;
        else
            tail->next = node;
        tail = node;
    }

    if (tail != NULL)
        tail->next = head; /* make it circular */
    return head;
}

void displayCoaches(Coach *head)
{
    Coach *cur;
    if (head == NULL)
    {
        printf("No coaches.\n");
        return;
    }

    cur = head;
    do
    {
        printf("C%d -> ", cur->coachNumber);
        cur = cur->next;
    } while (cur != head);
    printf("(back to C%d)\n", head->coachNumber);
}

Coach *getNextCoach(Coach *current)
{
    if (current == NULL)
        return NULL;
    return current->next;
}

int countCoaches(Coach *head)
{
    Coach *cur;
    int count = 0;
    if (head == NULL)
        return 0;

    cur = head;
    do
    {
        count++;
        cur = cur->next;
    } while (cur != head);
    return count;
}

void freeCoaches(Coach **head)
{
    Coach *cur, *next;
    if (head == NULL || *head == NULL)
        return;

    cur = (*head)->next;
    while (cur != *head)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
    free(*head);
    *head = NULL;
}
