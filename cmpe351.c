/* This is code developed by Nikita Mavrodiy */

#include "cmpe351.h"
#include <stdlib.h>
#include <stdio.h>

struct process {
    int burst;
    int priority;
    int arrival;
    int queue_id;
    struct process *next;
};

struct queue {
    int queue_id;
    struct process *processes;
    struct queue *next;
};

struct process * 
read_input(FILE *in)
{
    struct process *head = NULL;
    struct process *tail = NULL;

    int burst, priority, arrival, queue_id;

    while (fscanf(in, "%d:%d:%d:%d",
                  &burst, &priority, &arrival, &queue_id) == 4) {

        struct process *p = malloc(sizeof(struct process));
        if (!p) {
            perror("malloc");
            return head;
        }

        p->burst = burst;
        p->priority = priority;
        p->arrival = arrival;
        p->queue_id = queue_id;
        p->next = NULL;

        if (!head)
            head = p;
        else
            tail->next = p;

        tail = p;
    }

    return head;
}

struct queue *
get_queue(struct queue **head, int queue_id)
{
    struct queue *q = *head;

    while (q) {
        if (q->queue_id == queue_id)
            return q;
        q = q->next;
    }

    /* Not found: create new queue */
    q = malloc(sizeof(struct queue));
    if (!q) {
        perror("malloc");
        return NULL;
    }

    q->queue_id = queue_id;
    q->processes = NULL;
    q->next = *head;
    *head = q;

    return q;
}

struct queue *
group_by_queue(struct process *plist)
{
    struct queue *queues = NULL;

    while (plist) {
        struct process *next = plist->next;  /* save next */

        struct queue *q = get_queue(&queues, plist->queue_id);
        if (!q)
            return queues;

        plist->next = q->processes;
        q->processes = plist;

        plist = next;  /* move to original next */
    }

    return queues;
}



int
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("Input file");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        perror("Output file");
        fclose(in);
        return 1;
    }

    struct process *plist = read_input(in);

    /* Temporary debug output */
    struct queue *qlist = group_by_queue(plist);

    /* Temporary debug output */
    struct queue *q = qlist;
    while (q) {
        fprintf(out, "Queue %d:\n", q->queue_id);
        struct process *p = q->processes;
        while (p) {
            fprintf(out, "  %d:%d:%d:%d\n",
                    p->burst, p->priority, p->arrival, p->queue_id);
            p = p->next;
        }
        q = q->next;
    }

    fclose(in);
    fclose(out);
    return 0;
}
