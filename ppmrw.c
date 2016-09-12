/** ppmrw program for reading and writing images in ppm format
 * Author: Michael Gilbert
 * CS430 - Computer Graphics
 * Project 1
 * usage: ppmrw 3|6 <infile> <outfile>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "include/ppmrw.h"

int get_infile_size(FILE *fp) {
    if (fp == NULL) {
        perror("Error: ");
        return -1;
    }        
    // seek to end and return size
    fseek(fp, 0, 2);
    return ftell(fp);
}

void read_comments(FILE *fh, char **cmts) {
    // reads comments in a file into a char array
    char *line = NULL;      // temporary line reader
    size_t len = 0;            // temp line length
    size_t read;               // number of bytes read
    int ptr = 0;            // pointer to index of cmts variable

    if (fgetc(fh) == '#') {
        fseek(fh, -1, SEEK_CUR); // move ahead 1 byte
        while ((read = getline(&line, &len, fh)) != -1) {
            if (line[0] != '#') {
                fseek(fh, -1, SEEK_CUR);
                break;  // we've reached the end of the comments
            }
            cmts[ptr++] = line;
            printf("%s", line);
        }
        cmts[ptr] = NULL;
        free(line);
    }
    else {
        perror("Error: The starting position is not a comment");
    }
}

void read_header(FILE *fh, header *hdr) {
    header* hdr = malloc(sizeof(header));

    int i;      // iterator variable
    char c;     // temporary char read in from file
    boolean is_p3; // determines file type being P3 or P6

    // get file type
    c = fgetc(fh);
    if (c != 'P')
        perror("Error: first character must be 'P'");
    c = fgetc(fh);
    if (c == '3') {
        is_p3 = TRUE;
    }
    else if (c == '6') {
        is_p3 = FALSE;
    }
    else {
        perror("Error: No file type found in header");
    }
    
    if (is_p3) {
        hdr->file_type = 3;
        printf("file is P3\n");
    }
    else {
        hdr->file_type = 6;
        printf("file is P6\n");
    }
    if (fgetc(fh) != '\n')
        perror("Error: must be a newline after file type");
    
    printf("\n");
    fseek(fh, -1, SEEK_CUR);

    // basic test to get header info
    hdr->file_type = (int)fgetc(fh);
    
    // read in comments
    c = fgetc(fh);
    if (c == '#') {
        printf("there's a comment\n");
        fseek(fh, -1, SEEK_CUR);
        char *cmts[1024];
        read_comments(fh, cmts);
    }
    return hdr;
}

int main(int argc, char *argv[]){
    if (argc != 4) {
        perror("Error: ppmrw requires 3 arguments");
        return 1;
    }

    FILE *in_ptr;
    FILE *out_ptr;
    int infile_size;

    in_ptr = fopen(argv[2], "rb");
    //infile_size = get_infile_size(in_ptr);
    //printf("infile size is: %d\n", infile_size);

    // test get_header function
    header* hdr = get_header(in_ptr);
    return 0;
}
