/* ppmrw header file */
#ifndef PPMRW_H
#define PPMRW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE 1
#define MAX_SIZE 1024

/* variables and types */
typedef int8_t boolean;

typedef struct header_t {
    int file_type;
    char **comments;
    int width;
    int height;
    int max_color_val;
} header;

/* functions */
void read_header(FILE *fh, header *hdr);
char **read_comments(FILE *fh);

#endif
