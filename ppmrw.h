/* ppmrw header file */

/* variables and types */
typedef struct header_t {
    int file_type;
    char **comments;
    int width;
    int height;
    int max_color_val;
} header;

/* functions */
header get_header(FILE *fh);

