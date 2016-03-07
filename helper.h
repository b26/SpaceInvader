struct image {
    double *x;
    double *y;
    int size;
};

int draw (FILE *executable, struct image image, int erase_draw);
int end (FILE *executable);
int load_image (FILE *executable, const char name[], struct image image);
int update (FILE *executable, struct image *images);
int move (struct image image, double x, double y);
int rotate (struct image image);
double *gravity (struct image *images,double g);
void free_image (struct image image);
void clear_screen (FILE *executable);
