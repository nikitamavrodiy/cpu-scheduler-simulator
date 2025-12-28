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
    struct process *p = plist;
    while (p) {
        fprintf(out, "%d:%d:%d:%d\n",
                p->burst, p->priority, p->arrival, p->queue_id);
        p = p->next;
    }

    fclose(in);
    fclose(out);
    return 0;
}
