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

void
fcfs_schedule(struct queue *q, FILE *out)
{
    double total_wait = 0.0;
    int time = 0;
    int count = 0;

    /* First, find earliest arrival */
    struct process *p = q->processes;
    int earliest = -1;
    while (p) {
        if (earliest == -1 || p->arrival < earliest)
            earliest = p->arrival;
        p = p->next;
    }

    time = earliest;

    // print queue id and algorithm number 
    fprintf(out, "%d:1", q->queue_id);

    p = q->processes;
    while (p) {
        int wait = time - p->arrival;
        if (wait < 0)
            wait = 0;

        fprintf(out, ":%d", wait);

        total_wait += wait;
        time += p->burst;
        count++;

        p = p->next;
    }

    if (count > 0)
        fprintf(out, ":%.2f\n", total_wait / count);
    else
        fprintf(out, ":0.00\n");
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

    struct queue *q = qlist;
    while (q) {
        fcfs_schedule(q, out);
        q = q->next;    
    }

    fclose(in);
    fclose(out);
    return 0;
}
