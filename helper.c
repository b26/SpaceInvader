#define _XOPEN_SOURCE
#define LINE_SIZE 150

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"

int end(FILE *executable) {
    fprintf(executable, "pause 2\n");
    fprintf(executable, "end\n");
    return 0;
}

void free_image (struct image image) {
    free(image.x);
    free(image.y);
}
void clear_screen (FILE *executable) {
    fprintf(executable, "clearScreen\n");
}

double *gravity(struct image *images, double g) {
    double *velocity;
    int size = sizeof(struct image)/sizeof(images);
    velocity = (double*)calloc(2, sizeof(double));
    /* i is set to 1 to ignore landscape */
    for (int i = 1; i < size - 1; i++) {
        for (int j = 0; j < images[i].size; j++) {
            double *y = &(images[i].y[j]);
            *y += g;
        }
    }

    return velocity;
}

int update (FILE *executable, struct image *images) {
    //clear_screen(executable);
    int erase = 0;
    int draw_me = 1;
    double g = 19.8;
    rotate(images[1]);
    gravity(images, g);
    draw(executable, images[0], draw_me);
    draw(executable, images[1], draw_me);
    fprintf(executable, "pause 1\n");
    draw(executable, images[1], erase);
    gravity(images, g);
    move(images[1], 20, 30);
    rotate(images[1]);
    draw(executable, images[1], draw_me);
    fprintf(executable, "pause 1\n");
    draw(executable, images[1], erase);
    gravity(images, g);
    move(images[1], 30, 40);
    rotate(images[1]);
    draw(executable, images[1], draw_me);
    return 0;
}

int rotate (struct image image) {
    int j = 0;
    double *x;
    double *y;
    double degrees = -10.0; /*keystroke*/
    double angle = degrees * M_PI/180.0;
    /* set to 1 so it could skip landscape which is stored in images[0]*/
    for (j = 0; j < image.size; j++) {
        /*handle rotations*/
        x = &(image.x[j]);
        y = &(image.y[j]);
        *x = *x*cos(angle) - *y*sin(angle);
        *y = *x*sin(angle) + *y*cos(angle);
    }
    return 0;
}

int move (struct image image, double x, double y) {
    int i = 0;
    for (i = 0; i < image.size; i++) {
        image.x[i] += x;
        image.y[i] += y;
    }
    return 0;
}

int draw (FILE *executable, struct image image, int erase_draw) {
    char segment[] = "drawSegment";
    if (erase_draw == 0) {
        strcpy(segment, "eraseSegment");
    }
    int i = 0;
    int x1, y1, x2, y2 = 0;
    for (i = 0; i < image.size-2; i++) {
        x1 = image.x[i];
        y1 = image.y[i];
        x2 = image.x[i+1];
        y2 = image.y[i+1];
        fprintf(executable, "%s %ld %ld %ld %ld\n", segment, lround(x1), lround(y1),
                lround(x2), lround(y2));
    }
    return 0;
}

int load_image (FILE *executable, const char name[], struct image image) {
    FILE *fp;
    fp = fopen(name, "r");
    char Content[LINE_SIZE];
    int line_count = 0;
    double x;
    double y;
    while (!feof(fp)){
    int scan_result;
    fgets(Content, LINE_SIZE, fp);
    scan_result = sscanf(Content, "%lf%lf", &x, &y);
        if (scan_result == 2) {
            image.x[line_count] = x;
            image.y[line_count] = y;
            line_count += 1;
        }
    }
    fclose(fp);
    return 0;
}
