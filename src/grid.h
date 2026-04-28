#ifndef SWAYNAV_GRID_H
#define SWAYNAV_GRID_H

#include <gtk/gtk.h>

/* Selection rectangle */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} GridRect;

/* Output/monitor information */
typedef struct {
    int x;
    int y;
    int w;
    int h;
    char *name;
} OutputInfo;

/* Grid state */
typedef struct {
    GridRect selection;        /* Current selection rectangle */
    GridRect initial;          /* Initial full-screen rectangle */
    OutputInfo *outputs;       /* Monitor array */
    int noutputs;              /* Number of monitors */
    int current_output;        /* Current monitor index */

    int grid_cols;             /* Grid columns (default 2) */
    int grid_rows;             /* Grid rows (default 2) */
    int grid_nav_active;       /* Grid navigation mode */

    int center_cut_size;       /* For cursorzoom */
} GridState;

/* Initialize grid state for given outputs */
void grid_init(GridState *gs, OutputInfo *outputs, int noutputs);

/* Set selection to full screen of current output */
void grid_reset(GridState *gs);

/* Cut operations - shrink selection */
void grid_cut_up(GridState *gs, const char *value);
void grid_cut_down(GridState *gs, const char *value);
void grid_cut_left(GridState *gs, const char *value);
void grid_cut_right(GridState *gs, const char *value);

/* Move operations - shift selection */
void grid_move_up(GridState *gs, const char *value);
void grid_move_down(GridState *gs, const char *value);
void grid_move_left(GridState *gs, const char *value);
void grid_move_right(GridState *gs, const char *value);

/* Grid operations */
void grid_set(GridState *gs, int cols, int rows);
void grid_cell_select(GridState *gs, const char *value);
void grid_toggle_nav(GridState *gs);

/* Zoom operations */
void grid_windowzoom(GridState *gs, int win_x, int win_y, int win_w, int win_h);
void grid_cursorzoom(GridState *gs, int cursor_x, int cursor_y, int w, int h);

/* Get center point of selection */
void grid_get_center(GridState *gs, int *cx, int *cy);

/* Free resources */
void grid_free(GridState *gs);

#endif /* SWAYNAV_GRID_H */