#define _XOPEN_SOURCE 700
#define LINE_SIZE 150
#define LANDSCAPE_SIZE_MAX 30
#define SHIP_SIZE_MAX 9
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>
#include "helper.h"

/*Start Globals. These are needed throughout entire program*/
struct image *images;
double *vt;
FILE *executable;
double t = 0.05;
struct image landscape;
struct image ship;
struct image fuel;
struct itimerval timer;
const char black[] = "0 0 0";
const char red[] = "255 0 0";
const char blue[] = "0 0 255";
char *landscape_fp;
int running = 1;
int c;
int gameOver = 0;
int clear_main = 0;
int outside = 0;
int improvements = 0;
double g = 0;
double thrust = 0;
double fuel_level = 80;
int ship_size = 0;
int thruster_size = 3;
/* End Globals*/

int main (int argc, char **argv) {

    /* Handle Arguments. This will by default handle all arguments, in any order.*/
    handle_arguments(argc, argv);

    /*open sketchpad pipe*/
    open_sketchpad_pipe();

    /* loads menu and inits our images */
    start_game();

    /* starts the timer */
    start_timer();

    /* runs background tasks such as checking for collision*/
    background_tasks();

    /* closes all files, pipes and frees all memory*/
    end_game();

    return 0;
}


/* functions needed specific to this page*/

void handle_arguments(int argc, char **argv) {

    /* https://linuxprograms.wordpress.com/2012/06/22/c-getopt_long-example-accessing-command-line-arguments*/

    int opt = 0;
    static struct option long_options[] = {
        {"gravity", required_argument, 0, 'g'},
        {"thrusters", required_argument, 0, 't'},
        {"file_name", required_argument, 0, 'f'},
        {"improvements", no_argument, 0, 'i'},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    while ((opt = getopt_long(argc, argv,"g:t:f:i", long_options, &long_index )) != -1) {
        switch (opt) {
            case 'g' :
                g = atoi(optarg);
                break;
            case 't' :
                thrust = atoi(optarg);
                break;
            case 'f':
                landscape_fp = optarg;
                break;
            case 'i':
                /* activating improvements*/
                improvements = 1;
                break;
            default:
                printf("usage: ./lander -g [int] -t [int] -f [filename] [-i for improvemnts]\n");
                exit(EXIT_FAILURE);
        }
    }
}

void open_sketchpad_pipe() {
    const char exec_name[] = "java -jar Sketchpad.jar";
    executable = popen(exec_name, "w");
    if (executable == NULL) {
        fprintf(stderr, "Could not open pipe: %s\n", exec_name);
    }
}

void background_tasks() {
    while(running) {
        gameOver = collide(images, vt[1]);
        c = getch();
        if (c == ERR) { continue; }
        running = handle_mouse(c, running);
    }
}

void start_timer() {
    struct sigaction handler;
    handler.sa_handler = handle_timeout;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;

    if(sigaction(SIGALRM, &handler, NULL) < 0) {
        exit(EXIT_FAILURE);
    }
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 50000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 50000;

    if(setitimer(ITIMER_REAL, &timer, NULL) < 0) {
        exit(EXIT_FAILURE);
    }

}

void handle_timeout(int signal) {
    if (signal == SIGALRM) {
        if ((gameOver == 1) || (outside == 1)) {
            ship_size = 6;
            update(executable, images, t, vt, g, ship_size, improvements);
            timer.it_value.tv_sec = 0;
            timer.it_value.tv_usec = 0;
            timer.it_interval.tv_sec = 0;
            timer.it_interval.tv_usec = 0;
            if (setitimer (ITIMER_REAL, &timer, NULL) < 0) {
                exit(EXIT_FAILURE);
            }
        }
    }
    update_block();
}

int update_block() {
    sigset_t block_mask;
    /*set of previously blocked signals*/
    sigset_t old_mask;

    /* Add SIGALRM to blocked signals*/
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGALRM);

    /* blocked signals go inside &old_mask*/
    if (sigprocmask(SIG_BLOCK, &block_mask, &old_mask) < 0) {
        exit(EXIT_FAILURE);
    }

    /* Critical Code Goes In Here */
    update(executable, images, t, vt, g, ship_size, improvements);
    /* resets the ship_size in order to only draw the ship without thrusters*/
    ship_size = ship.size - thruster_size;
    outside = is_outside(images, ship_size);

    /* Unblock any signals by setting old_mask to NULL*/
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) {
        exit(EXIT_FAILURE);
    }

    return 0;
}

void start_game() {
    /* start ncurses*/
    init_ncurses();

    /* load menu */
    if (clear_main == 0) {
        mvprintw(10, 10, "Press any key to start.\n");
        mvprintw(11, 10, "(Then press arrow keys to rotate, space for thrust, 'q' to quit.)");
        clear_main = 1;
        getch();
        erase();
        mvprintw(10, 0, "left arrow key rotates counter-clockwise, right clockwise, space for thrust, q to quit.\n");
        refresh();
    }
    /* init game settings*/
    init_game();
}

void end_game() {
    end(executable);
    pclose(executable);
    free_image(landscape);
    free_image(ship);
    free_image(fuel);
    free(images);
    shutdown_ncurses();
}

int handle_mouse(int c, int running) {
    if (clear_main == 1) {
        switch( c ) {
            case KEY_LEFT:
                rotate(images[1], -10.0);
                break;
            case KEY_RIGHT:
                rotate(images[1], 10.0);
                break;
            case ' ':
                if (improvements == 1) {
                    if (fuel_level > 0) {
                        ship_size = 9;
                        thrusters(vt, thrust, t);
                        fuel_level -= 10;
                        images[2].x[2] = fuel_level;
                        images[2].x[1] = fuel_level;
                    }
                    else {
                        printw("out of fuel!!!\n");
                    }
                }
                else {
                    ship_size = 9;
                    thrusters(vt, thrust, t);
                }
                break;
            case 'q':
                running = 0;
                gameOver = 1;
                break;
            default:
                break;
        }
    }
    return running;
}

void init_ncurses() {
    int r;

    // start up the ncurses environment
    initscr(); // nothing to check, initscr exits on error

    // don't wait for enter for keyboard input
    r = cbreak(); assert( r != ERR );

    // don't echo keypresses to screen
    r = noecho(); assert( r != ERR );

    r = nonl(); assert( r != ERR );

    // turn cursor off
    r = curs_set( 0 ); assert( r != ERR );

    // don't worry about cursor position:w

    r = leaveok( stdscr, TRUE ); assert( r != ERR );

    // slower handling of ^C but saner output
    r = intrflush( stdscr, FALSE ); assert( r != ERR );

    // translate movement espace codes into single keys
    r = keypad( stdscr, TRUE ); assert( r != ERR );
}

void shutdown_ncurses() {
    endwin();
}

void init_game() {

    /*setup our images*/
    const char ship_file[] = "ship_middle.txt";
    const char fuel_file[] = "lineOne.txt";

    /* init our velocity*/
    vt = calloc(2, sizeof(double));
    vt[0] = 0.00;
    vt[1] = 0.00;

    images = calloc(10, sizeof(struct image));
    landscape.x = calloc(LANDSCAPE_SIZE_MAX, sizeof(double));
    landscape.y = calloc(LANDSCAPE_SIZE_MAX, sizeof(double));
    ship.x = calloc(SHIP_SIZE_MAX, sizeof(double));
    ship.y = calloc(SHIP_SIZE_MAX, sizeof(double));
    fuel.x = calloc(10, sizeof(double));
    fuel.y = calloc(10, sizeof(double));

    landscape.size = load_image(executable, landscape_fp, landscape);
    ship.size = load_image(executable, ship_file, ship);
    images[0] = landscape;
    images[1] = ship;
    ship_size = ship.size - thruster_size;

    /* make sure ship is pointing upwards*/
    rotate(images[1], 180);
    rotate(images[1], -10);
    rotate(images[1], 10);


    /* draw images depending on if improvements is enabled*/
    if (improvements == 1 ) {
        fuel.size = load_image(executable, fuel_file, fuel);
        images[2] = fuel;
        draw(executable, images[2], fuel.size, red);
        draw(executable, images[0], landscape.size, black);
        draw(executable, images[1], ship_size, blue);
    }
    else {
        draw(executable, images[0], landscape.size, black);
        draw(executable, images[1], ship_size, black);
    }
}

