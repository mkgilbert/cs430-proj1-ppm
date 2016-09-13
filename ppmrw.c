/** ppmrw program for reading and writing images in ppm format
 * Author: Michael Gilbert
 * CS430 - Computer Graphics
 * Project 1
 * usage: ppmrw 3|6 <infile> <outfile>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char **read_comments(FILE *fh) {
    // reads comments in a file into a char array
    char *line = NULL;      // temporary line reader
    size_t len = 0;            // temp line length
    size_t read;               // number of bytes read
    int ptr = 0;            // pointer to index of cmts variable
    char **cmts = malloc(sizeof(char*) * MAX_SIZE);

    if (fgetc(fh) == '#') {
        fseek(fh, -1, SEEK_CUR); // move ahead 1 byte
        while ((read = getline(&line, &len, fh)) != -1) {
            if (line[0] != '#') {
                // we've now read 1 too many lines. Back up
                fseek(fh, read*(-1), SEEK_CUR);
                break;  // we've reached the end of the comments
            }
            // allocate space for each line in cmts
            cmts[ptr] = malloc(sizeof(char) * strlen(line));
            strcpy(cmts[ptr], line);
            ptr++;
        }
        // null terminate after last line
        cmts[ptr] = NULL;
        free(line);
    }
    else {
        perror("Error: The starting position is not a comment");
    }
    return cmts;
}

void read_header(FILE *fh, header *hdr) {
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
    }
    else {
        hdr->file_type = 6;
    }
    if (fgetc(fh) != '\n') {
        perror("Error: must be a newline after file type");
    }
    
    // read in comments
    c = fgetc(fh);
    if (c == '#') {
        // go back one space to get to beginning of comment
        fseek(fh, -1, SEEK_CUR);

        char** cmts = read_comments(fh);
        if (cmts == NULL) {
            perror("Error: found a '#' but got no comments...");
        }
        // allocate space in hdr struct for the comments and copy them in
        hdr->comments = cmts; // don't allocate maybe? just point???
        // testing output
        //int8_t ptr = 0;
        //while (cmts[ptr] != NULL)
        //    printf("%s\n", cmts[ptr++]);
    }

    // read width and height
    fscanf(fh, "%d %d", &(hdr->width), &(hdr->height));
    // TODO: Error checking
    
    fscanf(fh, "%d", &(hdr->max_color_val));
    // TODO: Type Error checking
    if (hdr->max_color_val > 255)
        perror("Error: max color value must be <= 255");
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
    header *hdr = (header *)malloc(sizeof(header));
    read_header(in_ptr, hdr);
    printf("File type: P%d\n", hdr->file_type);
    printf("Comments: \n");
    int8_t ptr = 0;
    while (hdr->comments[ptr] != NULL)
        printf("%s", hdr->comments[ptr++]);
    printf("Width: %d\n", hdr->width);
    printf("Height: %d\n", hdr->height);
    printf("Max Color Value: %d\n", hdr->max_color_val);
    
    free(hdr->comments);
    free(hdr);
    fclose(in_ptr);
    return 0;
}
