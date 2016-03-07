#define _XOPEN_SOURCE
#define LINE_SIZE 150
#define VELOCITY_SIZE 2
#define SHIP_SIZE 6

#include <math.h>
#include <stdlib.h>
#include <ncurses.h>
#include <stdbool.h>
#include "helper.h"


/*we will rotate the ship relative to this direction*/
double direction = 270.0 * M_PI/180.0;


/* prints 'end' command into sketchpad to close the window without errors*/

int end(FILE *executable) {
    fprintf(executable, "end\n");
    return 0;
}

/* frees our allocated X and Y points*/
void free_image (struct image image) {
    free(image.x);
    free(image.y);
}

/* clears screen in sketchpad*/
void clear_screen (FILE *executable) {
    fprintf(executable, "clearScreen\n");
}

/* applies thrusters*/
void thrusters(double *v, double thrust, double t) {
    v[0] += thrust*t*(-1)*cos(direction);
    v[1] += thrust*t*sin(direction);
}

/* applies gravity to a given image*/
void gravity(struct image image, double t, double g) {
    for (int i = 0; i < image.size; i++) {
        image.y[i] += g*t;
    }
}

/* moves ship given velocity*/
int move_ship (struct image image, double *v) {
    for (int i = 0; i < image.size; i++) {
        image.x[i] += v[0];
        image.y[i] += v[1];
    }
    return 0;
}

/* updates our images*/
int update (FILE *executable, struct image *images, double t, double *vt, double g, int drawSize, int improvements) {
    const char black[] = "0 0 0";
    const char red[] = "255 0 0";
    const char blue[] = "0 0 255";
    clear_screen(executable);
    gravity(images[1], t, g);
    move_ship(images[1], vt);
    if (improvements == 1) {
        draw(executable, images[0], images[0].size, black);
        draw(executable, images[1], drawSize, blue);
        draw(executable, images[2], images[2].size, red);
    }
    else {
        draw(executable, images[0], images[0].size, black);
        draw(executable, images[1], drawSize, black);
    }
    return 0;
}

/* grabs the min max of a given X or Y point*/
double min_max(double *point) {
    double point_midpoint;
    double min = point[0];
    double max = point[0];
    for (int i = 0; i < SHIP_SIZE; i++) {
        if (point[i] > max) {
            max = point[i];
        }
        if (point[i] < min) {
            min = point[i];
        }
    }
    point_midpoint = ((min + max)/2);
    return point_midpoint;
}

/* grabs the midpoint of an image*/
double *get_midpoint(struct image image) {
    double *midpoint = calloc(2, sizeof(double));
    midpoint[0] = min_max(image.x);
    midpoint[1] = min_max(image.y);
    return midpoint;
}

/*rotate an image around the midpoint*/
int rotate (struct image image, double degrees) {
    double *x;
    double *y;
    double old_x;
    double *midpoint = get_midpoint(image);
    double angle = degrees * M_PI/180.0;
    direction -= angle;
    for (int j = 0; j < image.size; j++) {
        x = &(image.x[j]);
        y = &(image.y[j]);
        old_x = *x;
        *x = (midpoint[0] + (*x-midpoint[0])*cos(angle) - (*y-midpoint[1])*sin(angle));
        *y = (midpoint[1] + (*y-midpoint[1])*cos(angle) + (old_x-midpoint[0])*sin(angle));
    }
    /*free midpoint*/
    free(midpoint);
    return 0;
}


/* draws the image in sketchpad */
int draw (FILE *executable, struct image image, int drawSize, const char color[]) {
    char segment[] = "drawSegment";
    int x1, y1, x2, y2, i = 0;
    for (i = 0; i < drawSize-2; i++) {
        x1 = image.x[i];
        y1 = image.y[i];
        x2 = image.x[i+1];
        y2 = image.y[i+1];
        fflush(NULL);
        fprintf(executable, "setColor %s\n", color);
        fprintf(executable, "%s %ld %ld %ld %ld\n", segment, lround(x1), lround(y1),
                lround(x2), lround(y2));
        fflush(NULL);
    }
    return 0;
}

/* loads are image from a given file and returns line_count*/
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
    return line_count;
}


/*detects intersection http://alienryderflex.com/intersect*/

bool lineSegmentIntersection(
        double Ax, double Ay,
        double Bx, double By,
        double Cx, double Cy,
        double Dx, double Dy,
        double *X, double *Y) {

    int NO = 0;
    int YES = 1;

    double  distAB, theCos, theSin, newX, ABpos ;

    //  Fail if either line segment is zero-length.
    if ((Ax==Bx && Ay==By) || (Cx==Dx && Cy==Dy)) return NO;

    //  Fail if the segments share an end-point.
    if ((Ax==Cx && Ay==Cy) || (Bx==Cx && By==Cy)
            || ( Ax==Dx && Ay==Dy) || (Bx==Dx && By==Dy)) {
        return NO; }

    //  (1) Translate the system so that point A is on the origin.
    Bx-=Ax; By-=Ay;
    Cx-=Ax; Cy-=Ay;
    Dx-=Ax; Dy-=Ay;

    //  Discover the length of se`gment A-B.
    distAB=sqrt(Bx*Bx+By*By);

    //  (2) Rotate the system so that point B is on the positive X axis.
    theCos=Bx/distAB;
    theSin=By/distAB;
    newX=Cx*theCos+Cy*theSin;
    Cy  =Cy*theCos-Cx*theSin; Cx=newX;
    newX=Dx*theCos+Dy*theSin;
    Dy  =Dy*theCos-Dx*theSin; Dx=newX;

    //  Fail if segment C-D doesn't cross line A-B.
    if ((Cy<0. && Dy<0.) || (Cy>=0. && Dy>=0.)) return NO;

    //  (3) Discover the position of the intersection point along line A-B.
    ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

    //  Fail if segment C-D crosses line A-B outside of segment A-B.
    if ((ABpos<0.) || (ABpos>distAB)) return NO;

    //  Success.
    return YES;
}

/* detects collision and accounts for speed */
int collide(struct image *images, double vt) {
    int YES = 1;
    int NO = 0;
    for (int s = 0; s < 6; s++) {
        for (int l = 0; l < images[0].size-1; l++) {
            if (lineSegmentIntersection(
                images[1].x[s], images[1].y[s],
                images[1].x[s+1], images[1].y[s+1],
                images[0].x[l], images[0].y[l],
                images[0].x[l+1], images[0].y[l+1],
                images[1].x, images[1].y) == YES) {
                if ((images[0].y[l] == images[0].y[l+1]) && (vt < 2.00)
                    && sin(direction) >= 1)  {
                    erase();
                    mvprintw(10, 0, "left arrow key rotates counter-clockwise, right clockwise, space for thrust, q to quit.\n");
                    mvprintw(15, 10, "LANDED!!!\n");
                    refresh();
                    return YES;
                }
                else {
                    erase();
                    mvprintw(10, 0, "left arrow key rotates counter-clockwise, right clockwise, space for thrust, q to quit.\n");
                    mvprintw(15, 10, "CRASHED!!!\n");
                    refresh();
                    return YES;
                }
            }
        }
    }
    return NO;
}

/* detects if ship is outside the window*/
int is_outside (struct image *images, int ship_size) {
    for (int i = 0; i < ship_size; i++) {
        if ((images[1].x[i] < 0) || (images[1].x[i] > 640)
            || (images[1].y[i] < 0)) {
        return 1;
        }
    }
    return 0;
}

