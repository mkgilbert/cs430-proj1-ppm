/** ppmrw program for reading and writing images in ppm format
 * Author: Michael Gilbert
 * CS430 - Computer Graphics
 * Project 1
 * usage: ppmrw 3|6 <infile> <outfile>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "ppmrw.h"

int get_infile_size(FILE *fp) {
    int size;
    if (fp == NULL) {
        perror("Error: ");
        return -1;
    }        
    // seek to end and return size
    fseek(fp, 0, 2);
    return ftell(fp);
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
    infile_size = get_infile_size(in_ptr);
    printf("infile size is: %d\n", infile_size);
    return 0;
}
