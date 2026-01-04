/* This is code developed by Nikita Mavrodiy */

#include "cmpe351.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct process {
    int burst;
    int priority;
    int arrival;
    int queue_id;
    int done;               
    int wait;               
    struct process *next;
};

struct queue {
    int queue_id;
    struct process *head;
    struct process *tail;
    struct queue *next;
};

static int
earliest_arrival(struct queue *q)
{
    int min = -1;
    struct process *p = q->head;

    while (p) {
        if (min == -1 || p->arrival < min)
            min = p->arrival;
        p = p->next;
    }
    return (min == -1) ? 0 : min;
}

static void
reset_processes(struct queue *q)
{
    struct process *p = q->head;
    while (p) {
        p->done = 0;
        p->wait = 0;
        p = p->next;
    }
}

/* Truncate a positive double to two decimal places, return as string without trailing zeros */
static void
format_truncated_awt(double val, char *buf, size_t bufsz)
{
    if (val < 0.0) val = 0.0;
    /* truncate to two decimals (toward zero) */
    int t = (int)(val * 100.0); /* truncation */
    double truncated = t / 100.0;
    /* print with two decimals then strip trailing zeros and dot */
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%.2f", truncated);
    /* strip trailing zeros */
    int len = (int)strlen(tmp);
    while (len > 0 && tmp[len-1] == '0') {
        tmp[--len] = '\0';
    }
    if (len > 0 && tmp[len-1] == '.') {
        tmp[--len] = '\0';
    }
    /* copy to buf */
    strncpy(buf, tmp, bufsz-1);
    buf[bufsz-1] = '\0';
}

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
        p->done = 0;
        p->wait = 0;
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

/* FCFS: simulate timeline and store waits in process->wait, then print in input order */
void
schedule_fcfs(struct queue *q, FILE *out)
{
    int time = 0;
    int count = 0;
    double total_wait = 0.0;

    reset_processes(q);
    
    /* compute waits and accumulate */
    struct process *p = q->head;
    while (p) {
        if (time < p->arrival)
            time = p->arrival;

        int wait = time - p->arrival;
        p->wait = wait;

        total_wait += wait;
        time += p->burst;
        count++;

        p = p->next;
    }

    /* print results in original input order */
    fprintf(out, "%d:1", q->queue_id);
    p = q->head;
    while (p) {
        fprintf(out, ":%d", p->wait);
        p = p->next;
    }

    char buf[32];
    if (count > 0) {
        double avg = total_wait / count;
        format_truncated_awt(avg, buf, sizeof(buf));
        fprintf(out, ":%s\n", buf);
    } else {
        fprintf(out, ":0\n");
    }
}

/* SJF non-preemptive: simulate, store waits in process->wait, print in input order */
void
schedule_sjf(struct queue *q, FILE *out)
{
    int completed = 0, count = 0;
    double total_wait = 0.0;

    reset_processes(q);

    int time = earliest_arrival(q);

    /* count processes */
    struct process *p = q->head;
    while (p) {
        count++;
        p = p->next;
    }

    while (completed < count) {
        struct process *chosen = NULL;
        p = q->head;
        while (p) {
            if (!p->done && p->arrival <= time) {
                if (!chosen || p->burst < chosen->burst)
                    chosen = p;
            }
            p = p->next;
        }

        if (!chosen) {
            time++;
            continue;
        }

        int wait = time - chosen->arrival;
        if (wait < 0) wait = 0; /* safety, though timeline ensures non-negative */
        chosen->wait = wait;

        total_wait += wait;
        time += chosen->burst;
        chosen->done = 1;
        completed++;
    }

    /* print results in original input order */
    fprintf(out, "%d:2", q->queue_id);
    p = q->head;
    while (p) {
        fprintf(out, ":%d", p->wait);
        p = p->next;
    }

    char buf[32];
    if (count > 0) {
        double avg = total_wait / count;
        format_truncated_awt(avg, buf, sizeof(buf));
        fprintf(out, ":%s\n", buf);
    } else {
        fprintf(out, ":0\n");
    }
}

/* Priority non-preemptive: simulate, store waits, print in input order */
void
schedule_priority(struct queue *q, FILE *out)
{
    int completed = 0, count = 0;
    double total_wait = 0.0;

    reset_processes(q);

    int time = earliest_arrival(q);

    /* count processes */
    struct process *p = q->head;
    while (p) {
        count++;
        p = p->next;
    }

    while (completed < count) {
        struct process *chosen = NULL;
        p = q->head;
        while (p) {
            if (!p->done && p->arrival <= time) {
                if (!chosen || p->priority < chosen->priority)
                    chosen = p;
            }
            p = p->next;
        }

        if (!chosen) {
            time++;
            continue;
        }

        int wait = time - chosen->arrival;
        if (wait < 0) wait = 0;
        chosen->wait = wait;

        total_wait += wait;
        time += chosen->burst;
        chosen->done = 1;
        completed++;
    }

    /* print results in original input order */
    fprintf(out, "%d:3", q->queue_id);
    p = q->head;
    while (p) {
        fprintf(out, ":%d", p->wait);
        p = p->next;
    }

    char buf[32];
    if (count > 0) {
        double avg = total_wait / count;
        format_truncated_awt(avg, buf, sizeof(buf));
        fprintf(out, ":%s\n", buf);
    } else {
        fprintf(out, ":0\n");
    }
}

void
run_schedulers(struct queue *queues, FILE *out)
{
    struct queue *q = queues;
    while (q) {
        schedule_fcfs(q, out);
        schedule_sjf(q, out);
        schedule_priority(q, out);
        q = q->next;
    }
}

int
main(int argc, char *argv[])
{
    const char *input_file = NULL;
    const char *output_file = NULL;

    // parse simple flags: -f input -o output (or fallback to argv[1] argv[2])
    if (argc == 3) {
        input_file = argv[1];
        output_file = argv[2];
    } else {
        for (int i=1; i<argc; i++) {
            if (strcmp(argv[i], "-f")==0 && i+1<argc) input_file = argv[++i];
            else if (strcmp(argv[i], "-o")==0 && i+1<argc) output_file = argv[++i];
        }
    }

    if (!input_file || !output_file) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(input_file, "r");
    if (!in) { 
        perror("Input file"); 
        return 1;
    }

    FILE *out = fopen(output_file, "w");
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

    /* Print output file to screen */
    out = fopen(output_file, "r");
    if (out) {
        char line[512];
        while (fgets(line, sizeof(line), out) != NULL) {
            fputs(line, stderr);
        }
        fclose(out);
    }

    return 0;
}
