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


int check_for_comments(FILE *fh, char c) {
    /* jumps over comments and onto the next line, then recursively checks again */
    // skip any leading white space
    while (isspace(c) && c != EOF) { c = fgetc(fh); }

    // base case, current char, c, is not a pound sign
    if (c != '#') {
        fseek(fh, -1, SEEK_CUR); // backup one character
        return 0;
    }
    else { // c is a comment, so read to end of line
        while (c != '\n' && c != EOF) {
            c = fgetc(fh);
        }
        if (c == EOF) {
            perror("Error: check_for_comments: Premature end of file");
            return -1;
        }
        else { // c is '\n', so grab the next char and check recursively
            return check_for_comments(fh, fgetc(fh));
        }
    }
}

int check_for_newline(char c) {
    if (!isspace(c)) {
        perror("Error: check_for_newline: missing newline or space");
        return -1;
    }
    return 0;
}

int read_header(FILE *fh, header *hdr) {
    int ret_val;
    char c;     // temporary char read in from file
    boolean is_p3; // determines file type being P3 or P6

    // get file type
    c = fgetc(fh);
    if (c != 'P') {
        perror("Error: read_header: Invalid ppm file. First character is not 'P'");
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
        perror("Error: read_header: Unsupported magic number found in header");
        return -1;
    }
    
    if (is_p3) {
        hdr->file_type = 3;
    }
    else {
        hdr->file_type = 6;
    }
    ret_val = check_for_newline(fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: No separator found after magic number");
        return -1;
    }
    ret_val = check_for_comments(fh, fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: Problem reading comment after magic number");
        return -1;
    }

    // read width
    fscanf(fh, "%d", &(hdr->width));
    if (hdr->width <= 0 || hdr->width == EOF) {
        perror("Error: read_header: Image width not found");
        return -1;
    }
    ret_val = check_for_newline(fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: No separator found after width");
        return -1;
    }
    ret_val = check_for_comments(fh, fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: Problem reading comment after width");
        return -1;
    }

    // read height
    ret_val = fscanf(fh, "%d", &(hdr->height));
    if (ret_val <= 0 || ret_val == EOF) {
        perror("Error: read_header: Image height not found");
        return -1;
    }
    ret_val = check_for_newline(fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: No separator found after height");
        return -1;
    }
    ret_val = check_for_comments(fh, fgetc(fh));
    if (ret_val < 0) {
        perror("Error: read_header: Problem reading comment after height");
        return -1;
    }
    
    // read max color value
    ret_val = fscanf(fh, "%d", &(hdr->max_color_val));
    if (ret_val <= 0 || ret_val == EOF) {
        perror("Error: read_header: Max color value not found");
        return -1;
    }
    // check bounds on max color value
    if (hdr->max_color_val > 255 || hdr->max_color_val < 0) {
        perror("Error: max color value must be >= 0 and <= 255");
        return -1;
    }

    return 0;
}

int bytes_left(FILE *fh) {
    // returns the number of bytes left in a file
    int bytes;
    int pos = ftell(fh);    // get current pointer
    fseek(fh, 0, SEEK_END);
    int end = ftell(fh);
    bytes = end - pos;
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
    // read all remaining data from image into buffer
    int b = bytes_left(fh);
    // check for error reading bytes_left()
    if (b < 0) {
        perror("Error: read_p6_data: Problem reading remaining bytes in image");
        return -1;
    }

    // create temp buffer for image data + 1 for null char
    unsigned char data[b+1];
    unsigned char *data_p = data;
    int read;

    // read the rest of the file and check that what remains is the right size
    if ((read = fread(data, 1, b, fh)) < 0) {
        perror("Error: read_p6_data: fread() returned an error when reading data");
        return -1;
    }

    // double check number of bytes actually read is correct
    if (read < b || read > b) {
        perror("Error: read_p6_data: image data doesn't match header dimensions");
        return -1;
    }
    // null terminate the buffer
    data[b] = '\0';

    int i, j, k;        // loop variables
    unsigned char num;  // build a number from chars read in from file

    // loop through buffer and populate RGBPixel array
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            RGBPixel px;
            for (k=0; k<3; k++) {
                // check that we haven't read more than what is available
                if (*data_p == '\0') {
                    perror("Error: read_p6_data: Image data is missing or header dimensions are wrong");
                    return -1;
                }
                num = *data_p++;
                if (num < 0 || num > 255) {
                    perror("Error: read_p6_data: found a pixel value out of range");
                    return -1;
                }

                if (k == 0) {
                    px.r = num;
                }
                else if (k == 1) {
                    px.g = num;
                }
                else {
                    px.b = num;
                }
            }
            pixmap[i * width + j] = px;
        }
    }
    // check if there's still data left
    if (*data_p != '\0') {
        perror("Error: read_p6_data: Extra image data was found in file");
        return -1;
    }
    return 0;
}

int read_p3_data(FILE *fh, RGBPixel *pixmap, int width, int height) {
    
    // read all remaining data from image into buffer
    int b = bytes_left(fh); 
    // check for error reading bytes_left()
    if (b < 0) {
        perror("Error: read_p3_data: reading remaining bytes");
        return -1;
    }

    // create temp buffer for image data + 1 for null char
    char data[b+1];
    char *data_p = data;
    int read;

    // read the rest of the file and check that what remains is the right size
    if ((read = fread(data, 1, b, fh)) < 0) {
        perror("Error: read_p3_data: fread returned an error when reading data");
        return -1;
    }
    
    // double check number of bytes actually read is correct
    if (read < b || read > b) {
        perror("Error: read_p3_data: image data doesn't match header dimensions");
        return -1;
    }
    // null terminate the buffer
    data[b] = '\0';
    // make sure we're not starting at a space or newline
    while (isspace(*data_p) && (*data_p != '\0')) { data_p++; };
    
    int i, j, k;        // loop variables
    int ptr;            // current index of the num array
    char num[4];        // holds string repr. of a 0-255 value
    int counter = 0;
    // loop through buffer and populate RGBPixel array
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            RGBPixel px;
            printf("counter: %d\n", counter++);
            for (k=0; k<3; k++) {
                ptr = 0;
                while (TRUE) {
                    // check that we haven't read more than what is available
                    printf("'%c'\n", *data_p);
                    if (*data_p == '\0') {
                        perror("Error: read_p3_data: Image data is missing or header dimensions are wrong");
                        return -1;
                    }
                    if (isspace(*data_p)) {
                        *(num + ptr) = '\0';
                        while (isspace(*data_p) && (*data_p != '\0')) { 
                            data_p++; 
                        }
                        break;
                    }
                    else {
                        *(num + ptr) = *data_p++;
                        ptr++;
                    }
                }

                if (atoi(num) < 0 || atoi(num) > 255) {
                    perror("Error: read_p3_data: found a pixel value out of range");
                    return -1;
                }

                if (k == 0) {
                    px.r = atoi(num);
                    printf("r: %d ", px.r);
                }
                else if (k == 1) {
                    px.g = atoi(num);
                    printf("g: %d ", px.g);
                }
                else {
                    px.b = atoi(num);
                    printf("b: %d\n", px.b);
                }
                pixmap[i * width + j] = px;
            }
        }
    }

    // skip any white space that may remain at the end of the data
    while (isspace(*data_p) && (*data_p != '\0')) { data_p++; };
    
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    // check if there's still data left
    if (*data_p != '\0') {
        for (i=0; i<500; i++){
            printf("%c\n", *data_p++);
        }
        printf("last spot: %c\n", *data_p);
        perror("Error: read_p3_data: Extra image data was found in file");
        return -1;
    }
    return 0;
}

int write_p3_data(FILE *fh, image *img) {
    int i,j;
    for (i=0; i<(img->height); i++) {
        for (j=0; j<(img->width); j++) {
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
        perror("Error: main: ppmrw requires 3 arguments");
        return 1;
    }

    FILE *in_ptr;
    FILE *out_ptr;
    int ret_val;
    char c;

    in_ptr = fopen(argv[2], "rb");
    out_ptr = fopen(argv[3], "wb");
    
    if (in_ptr == NULL) {
        perror("Error: main: Input file can't be opened");
        return 1;
    }
    if (out_ptr == NULL) {
        perror("Error: main: Output file can't be opened");
        return 1;
    }

    // allocate space for header information
    header *hdr = (header *)malloc(sizeof(header));

    // read header of input file
    ret_val = read_header(in_ptr, hdr);
    
    if (ret_val < 0) {
        perror("Error: main: Problem reading header");
        return 1;
    }
    // read one more byte before reading data (should be a space-type character)
    c = fgetc(in_ptr);
    if (!isspace(c)) {
        perror("Error: main: No delimiter after header in input file");
        return 1;
    }

    // store the file type of the origin file so we know what we're converting from
    int origin_file_type = hdr->file_type;
    // change the header file type to what the destinationn file type should be
    if (atoi(argv[1]) == 3) {
        hdr->file_type = 3;
    }
    else if (atoi(argv[1]) == 6) {
        hdr->file_type = 6;
    }
    else {
        perror("Error: main: invalid file type specified. Choices: 3|6");
        return -1;
    }

    // write header info to output file
    ret_val = write_header(out_ptr, hdr);
    if (ret_val < 0) {
        perror("Error: main: Problem writing header to output file");
        return -1;
    }
    printf("successfully wrote header.\n");

    // create img struct to store relevant image info
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
    print_pixels(img.pixmap, img.width, img.height);
    
    // write image data to destination file
    if (atoi(argv[1]) == 3)
        ret_val = write_p3_data(out_ptr, &img);
    else
        ret_val = write_p6_data(out_ptr, &img);
    
    if (ret_val < 0) {
        perror("Error: main: Problem writing image data to output file");
        return -1;
    }
    printf("successfully wrote image data\n");
    
    // cleanup
    free(img.pixmap);
    free(hdr);
    fclose(in_ptr);
    fclose(out_ptr);
    return 0;
}
