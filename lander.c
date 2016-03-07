#define _XOPEN_SOURCE
#define LINE_SIZE 150
/*FIXME need to remove some of these*/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"


int main (int argc, char **argv) {
    const char exec_name[] = "java -jar Sketchpad.jar";
    //const char exec_name[] = "java -jar Sketchpad.jar -d";
    //FILE *landscape_fp;
    //FILE *ship_fp;
    FILE *executable;

    /* FIXME  handle argument length here */
    if (argc != 2) {
        fprintf(stderr, "Remember to load landscape.txt\n");
        return 0;
    }

    //landscape_fp = fopen(argv[1], "r");

    executable = popen(exec_name, "w");
    if (executable == NULL) {
        fprintf(stderr, "Could not open pipe: %s\n", exec_name);
    }

    /*variables*/
    struct image landscape;
    struct image ship;
    struct image *images;
    char const ship_file[] = "ship.txt";
    landscape.size = 7;
    ship.size = 6;
    images = (struct image*)calloc(2, sizeof(struct image));
    landscape.x = (double*)calloc(landscape.size, sizeof(double));
    landscape.y = (double*)calloc(landscape.size, sizeof(double));
    ship.x = (double*)calloc(ship.size, sizeof(double));
    ship.y = (double*)calloc(ship.size, sizeof(double));

    /*save landscape data*/

    /*draw landscape*/
    load_image(executable, argv[1], landscape);
    load_image(executable, ship_file, ship);

    /* save landscape and ship into images array*/
    images[0] = landscape;
    images[1] = ship;

    /* this handles updating all images*/
    update(executable, images);

    /*call before exiting*/
    end(executable);
    free_image(landscape);
    free_image(ship);
    pclose(executable);

    return 0;
}












