/* This is code developed by Nikita Mavrodiy */
#include "cmpe351.h"
#include <stdlib.h>
#include <stdio.h>

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

    /* Temporary message */
    fprintf(out, "OK\n");

    fclose(in);
    fclose(out);

    return 0;
}
