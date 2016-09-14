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
#include <ctype.h>
#include <unistd.h>
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

int read_header(FILE *fh, header *hdr) {
    char c;     // temporary char read in from file
    boolean is_p3; // determines file type being P3 or P6

    // get file type
    c = fgetc(fh);
    if (c != 'P') {
        perror("Error: first character must be 'P'");
        return -1;
    }
    c = fgetc(fh);
    if (c == '3') {
        is_p3 = TRUE;
    }
    else if (c == '6') {
        is_p3 = FALSE;
    }
    else {
        perror("Error: No file type found in header");
        return -1;
    }
    
    if (is_p3) {
        hdr->file_type = 3;
    }
    else {
        hdr->file_type = 6;
    }
    // TODO: change to isspace()
    if (fgetc(fh) != '\n') {
        perror("Error: must be a newline after file type");
        return -1;
    }
    
    // read in comments
    c = fgetc(fh);
    if (c == '#') {
        // go back one space to get to beginning of comment
        fseek(fh, -1, SEEK_CUR);

        char** cmts = read_comments(fh);
        if (cmts == NULL) {
            perror("Error: found a '#' but got no comments...");
            return -1;
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
    if (hdr->max_color_val > 255) {
        perror("Error: max color value must be <= 255");
        return -1;
    }

    return 0;
}

int bytes_left(FILE *fh) {
    // returns the number of bytes left in a file
    int bytes;
    int pos = ftell(fh);    // get current pointer
    printf("pos: %d\n", pos);
    fseek(fh, 0, SEEK_END);
    int end = ftell(fh);
    printf("end: %d\n", end);
    bytes = end - pos;
    printf("number of bytes left: %d\n", bytes);
    fseek(fh, pos, SEEK_SET); // put the pointer back
    if (bytes <= 0) {
        perror("Error: bytes_left: bytes remaining <= 0");
        return -1;
    }
    
    return bytes;
}

int write_p6_data(FILE *fh, image *img) {
    int i,j;
    for (i=0; i<(img->height); i++) {
        for (j=0; j<(img->width); j++) {
            fwrite(&(img->pixmap[i * img->width + j].r), 1, 1, fh);
            fwrite(&(img->pixmap[i * img->width + j].g), 1, 1, fh);
            fwrite(&(img->pixmap[i * img->width + j].b), 1, 1, fh);
        }
    }
    return 0;
}

int read_p6_data(FILE *fh, RGBPixel *pixmap, int width, int height) {
    // reads p6 data and stores in buffer
    int i, j, k;
    int ptr;
    unsigned char num; // build a number from chars read in from file
    int counter = 0;
    // read all remaining data from image into buffer
    int b = bytes_left(fh);     // get how many bytes to read to get to end
    if (b < 0) {
        perror("Error: read_p3_data: reading remaining bytes");
        return -1;
    }

    unsigned char *data = malloc(sizeof(unsigned char)*b);
    int read;
    if ((read = fread(data, b, 1, fh)) < 0) {
        perror("Error: fread() returned an error when reading data");
        return -1;
    }
    data[b] = '\0';
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            counter++;
            RGBPixel px;
            pixmap[i * width + j] = px;
            for (k=0; k<3; k++) {
                num = *data++;
                if (num < 0 || num > 255) {
                    perror("Error: found a pixel value out of range");
                    return -1;
                }

                if (k == 0) {
                    px.r = num;
                    printf("r: %d\n", num);
                }
                else if (k == 1) {
                    px.g = num;
                    printf("g: %d\n", num);
                }
                else {
                    px.b = num;
                    printf("b: %d\n", num);
                }
            }
        }
    }
    return 0;
}

int read_p3_data(FILE *fh, RGBPixel *pixmap, int width, int height) {
    int i, j, k;
    int ptr;
    char num[4]; // build a number from chars read in from file
    char c = '\0';      // holds current byte being read
    int counter = 0;
    // read all remaining data from image into buffer
    int b = bytes_left(fh);     // get how many bytes to read to get to end
    if (b < 0) {
        perror("Error: read_p3_data: reading remaining bytes");
        return -1;
    }

    char *data = malloc(sizeof(char)*b);
    int read;
    if ((read = fread(data, b, 1, fh)) < 0) {
        perror("Error: fread returned an error when reading data");
        return -1;
    }
    data[b] = '\0';
    // make sure we're not starting at a space
    while (isspace(*data) && (*data != '\0')) { data++; };

    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            counter++;
            RGBPixel px;
            pixmap[i * width + j] = px;
            for (k=0; k<3; k++) {
                ptr = 0;
                while (TRUE) {
                    if (isspace(*data)) {
                        printf("found space '%c'\n", *data);
                        *(num + ptr) = '\0';
                        data++;
                        break;
                    }
                    else {
                        printf("found num %c\n", *data);
                        *(num + ptr) = *data++;
                        ptr++;
                    }
                }

                if (atoi(num) < 0 || atoi(num) > 255) {
                    perror("Error: found a pixel value out of range");
                    return -1;
                }

                if (k == 0) {
                    px.r = atoi(num);
                    printf("r: %d\n", atoi(num));
                }
                else if (k == 1) {
                    px.g = atoi(num);
                    printf("g: %d\n", atoi(num));
                }
                else {
                    px.b = atoi(num);
                    printf("b: %d\n", atoi(num));
                }
            }
        }
    }
    return 0;
}

int write_p3_data(FILE *fh, image *img) {
    int i,j;
    int counter = 0;
    for (i=0; i<(img->height); i++) {
        for (j=0; j<(img->width); j++) {
            counter++;
            fprintf(fh, "%d ", img->pixmap[i * img->width + j].r);
            fprintf(fh, "%d ", img->pixmap[i * img->width + j].g);
            fprintf(fh, "%d\n", img->pixmap[i * img->width + j].b);
        }
    }
    return 0;
}

int write_header(FILE *fh, header *hdr) {
    int ret_val = 0;
    ret_val = fputs("P", fh);
    if (ret_val < 0) {
        return -1;
    }
    ret_val = fprintf(fh, "%d", hdr->file_type);
    if (ret_val < 0) {
        return -2;
    }
    ret_val = fputs("\n", fh);
    if (ret_val < 0) {
        return -3;
    }
    ret_val = fprintf(fh, "%d %d\n%d\n", hdr->width,
                                         hdr->height,
                                         hdr->max_color_val);
    if (ret_val < 0) {
        return -4;
    }
    printf("successfully wrote header.\n");
    return ret_val;
}

void print_pixels(RGBPixel *pixmap, int width, int height) {
    int i,j;
    int counter = 0;
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            counter++;
            printf("r: %d, ", pixmap[i * width + j].r);
            printf("g: %d ,", pixmap[i * width + j].g);
            printf("b: %d\n", pixmap[i * width + j].b);
        }
    }
    printf("print_pixels count: %d\n", counter);
}

int main(int argc, char *argv[]){
    if (argc != 4) {
        perror("Error: ppmrw requires 3 arguments");
        return 1;
    }

    FILE *in_ptr;
    FILE *out_ptr;
    int ret_val;
    char c;

    in_ptr = fopen(argv[2], "rb");
    out_ptr = fopen(argv[3], "wb");

    // test get_header function
    header *hdr = (header *)malloc(sizeof(header));
    read_header(in_ptr, hdr);

    // read one more byte before reading data (should be a space-type character)
    c = fgetc(in_ptr);
    if (!isspace(c)) {
        perror("Error: No delimiter after header in input file");
        return -1;
    }

    printf("File type: P%d\n", hdr->file_type);
    printf("Comments: \n");
    int8_t ptr = 0;
    while (hdr->comments[ptr] != NULL)
        printf("%s", hdr->comments[ptr++]);
    printf("Width: %d\n", hdr->width);
    printf("Height: %d\n", hdr->height);
    printf("Max Color Value: %d\n", hdr->max_color_val);
   
    int origin_file_type = hdr->file_type;
    // determine which ppm version to make and change the header
    if (atoi(argv[1]) == 3) {
        hdr->file_type = 3;
    }
    else if (atoi(argv[1]) == 6) {
        hdr->file_type = 6;
    }
    else {
        perror("Error: invalid file type specified. Choices: 3|6");
        return -1;
    }

    ret_val = write_header(out_ptr, hdr);
    if (ret_val < 0) {
        perror("Error: main: Problem writing header to output file");
        return -1;
    }

    // create image struct
    image img;
    img.width = hdr->width;
    img.height = hdr->height;
    img.pixmap = malloc(sizeof(RGBPixel) * img.width * img.height);
    
    // read image data
    if (origin_file_type == 3)
        ret_val = read_p3_data(in_ptr, img.pixmap, img.width, img.height);
    else
        ret_val = read_p6_data(in_ptr, img.pixmap, img.width, img.height);
    
    if (ret_val < 0) {
        perror("Error: main: Problem reading image data");
        return -1;
    }

    // testing
    //print_pixels(img.pixmap, img.width, img.height);
    
    // write image data
    if (atoi(argv[1]) == 3)
        ret_val = write_p3_data(out_ptr, &img);
    else
        ret_val = write_p6_data(out_ptr, &img);
    
    if (ret_val < 0) {
        perror("Error: main: Problem writing image data to output file");
        return -1;
    }
    // cleanup
    free(hdr->comments);
    free(img.pixmap);
    free(hdr);
    fclose(in_ptr);
    fclose(out_ptr);
    return 0;
}
