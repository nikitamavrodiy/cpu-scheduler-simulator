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
    struct process *head;
    struct process *tail;
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
find_or_create_queue(struct queue **queues, int queue_id)
{
    struct queue *q = *queues;
    struct queue *last = NULL;

    while (q) {
        if (q->queue_id == queue_id)
            return q;
        last = q;
        q = q->next;
    }

    q = malloc(sizeof(struct queue));
    if (!q) {
        perror("malloc");
        return NULL;
    }

    q->queue_id = queue_id;
    q->head = NULL;
    q->tail = NULL;
    q->next = NULL;

    if (!*queues)
        *queues = q;
    else
        last->next = q;

    return q;
}

struct queue *
group_processes_by_queue(struct process *plist)
{
    struct queue *queues = NULL;

    while (plist) {
        struct process *next = plist->next;
        plist->next = NULL;

        struct queue *q = find_or_create_queue(&queues, plist->queue_id);
        if (!q)
            return queues;

        if (!q->head) {
            q->head = plist;
            q->tail = plist;
        } else {
            q->tail->next = plist;
            q->tail = plist;
        }

        plist = next;
    }

    return queues;
}

void
schedule_fcfs(struct queue *q, FILE *out)
{
    int time = 0;
    int count = 0;
    double total_wait = 0.0;

    fprintf(out, "%d:1", q->queue_id);

    struct process *p = q->head;
    while (p) {
        if (time < p->arrival)
            time = p->arrival;

        int wait = time - p->arrival;
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

void
run_schedulers(struct queue *queues, FILE *out)
{
    struct queue *q = queues;
    while (q) {
        schedule_fcfs(q, out);
        q = q->next;
    }
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
    struct queue *queues = group_processes_by_queue(plist);

    run_schedulers(queues, out);

    fclose(in);
    fclose(out);
    return 0;
}
