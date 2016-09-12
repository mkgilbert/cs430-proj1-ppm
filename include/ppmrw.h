/* ppmrw header file */
#include <stdio.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1

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
void read_comments(FILE *fh, char **cmts);

