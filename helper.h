/* for bool types*/
#include <stdbool.h>

struct image {
    double *x;
    double *y;
    int size;
};

int draw (FILE *executable, struct image image, int drawSize, const char color[]);
int end (FILE *executable);
int load_image (FILE *executable, const char name[], struct image image);
int update (FILE *executable, struct image *images, double t,double *vt, double g, int drawSize, int improvements);
int move_ship(struct image image, double *v);
int rotate (struct image image, double degrees);
int handle_mouse(int c, int running);
int update_block();
int collide(struct image *images, double vt);
int is_outside (struct image *images, int ship_size);
void init_ncurses();
void shutdown_ncurses();
void thrusters(double *vt, double thrust, double t);
void free_image (struct image image);
void clear_screen (FILE *executable);
void handle_timeout (int signal);
void init_game();
void end_game();
void start_timer();
void background_tasks();
void open_sketchpad_pipe();
void handle_arguments(int argc, char **argv);
void gravity(struct image image, double t, double g);
void start_game();
double *get_midpoint(struct image image);
double min_max(double *point);
bool linesegmentintersection(double ax, double ay, double bx, double by,
    double cx, double cy, double dx, double dy, double *x, double *y);



