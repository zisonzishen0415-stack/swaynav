#include "grid.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void grid_init(GridState *gs, OutputInfo *outputs, int noutputs) {
    gs->outputs = outputs;
    gs->noutputs = noutputs;
    gs->current_output = 0;
    gs->grid_cols = 2;
    gs->grid_rows = 2;
    gs->grid_nav_active = 0;
    gs->center_cut_size = 200;

    /* Set initial selection to first output */
    if (noutputs > 0) {
        gs->initial.x = outputs[0].x;
        gs->initial.y = outputs[0].y;
        gs->initial.w = outputs[0].w;
        gs->initial.h = outputs[0].h;
        grid_reset(gs);
    }
}

void grid_reset(GridState *gs) {
    OutputInfo *out = &gs->outputs[gs->current_output];
    gs->selection.x = out->x;
    gs->selection.y = out->y;
    gs->selection.w = out->w;
    gs->selection.h = out->h;
}

void grid_cut_up(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.h, value, 0.5f);
    gs->selection.h -= amount;
}

void grid_cut_down(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.h, value, 0.5f);
    gs->selection.y += amount;
    gs->selection.h -= amount;
}

void grid_cut_left(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.w, value, 0.5f);
    gs->selection.w -= amount;
}

void grid_cut_right(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.w, value, 0.5f);
    gs->selection.x += amount;
    gs->selection.w -= amount;
}

void grid_move_up(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.h, value, 1.0f);
    gs->selection.y -= amount;
    if (gs->selection.y < gs->outputs[gs->current_output].y) {
        gs->selection.y = gs->outputs[gs->current_output].y;
    }
}

void grid_move_down(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.h, value, 1.0f);
    OutputInfo *out = &gs->outputs[gs->current_output];
    gs->selection.y += amount;
    if (gs->selection.y + gs->selection.h > out->y + out->h) {
        gs->selection.y = out->y + out->h - gs->selection.h;
    }
}

void grid_move_left(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.w, value, 1.0f);
    gs->selection.x -= amount;
    if (gs->selection.x < gs->outputs[gs->current_output].x) {
        gs->selection.x = gs->outputs[gs->current_output].x;
    }
}

void grid_move_right(GridState *gs, const char *value) {
    int amount = parse_value(gs->selection.w, value, 1.0f);
    OutputInfo *out = &gs->outputs[gs->current_output];
    gs->selection.x += amount;
    if (gs->selection.x + gs->selection.w > out->x + out->w) {
        gs->selection.x = out->x + out->w - gs->selection.w;
    }
}

void grid_set(GridState *gs, int cols, int rows) {
    if (cols > 0) gs->grid_cols = cols;
    if (rows > 0) gs->grid_rows = rows;
}

void grid_cell_select(GridState *gs, const char *value) {
    int col = 0, row = 0, num = 0;

    /* Try COLxROW format first */
    if (strchr(value, 'x') != NULL) {
        sscanf(value, "%dx%d", &col, &row);
        if (col > 0 && row > 0) {
            col--; row--; /* Convert to 0-indexed */
        }
    } else {
        /* Try numeric format */
        sscanf(value, "%d", &num);
        if (num > 0) {
            num--; /* Convert to 0-indexed */
            row = num / gs->grid_cols;
            col = num % gs->grid_cols;
        }
    }

    if (col >= 0 && col < gs->grid_cols && row >= 0 && row < gs->grid_rows) {
        int cell_w = gs->selection.w / gs->grid_cols;
        int cell_h = gs->selection.h / gs->grid_rows;

        gs->selection.x += col * cell_w;
        gs->selection.y += row * cell_h;
        gs->selection.w = cell_w;
        gs->selection.h = cell_h;
    }
}

void grid_toggle_nav(GridState *gs) {
    gs->grid_nav_active = !gs->grid_nav_active;
}

void grid_windowzoom(GridState *gs, int win_x, int win_y, int win_w, int win_h) {
    gs->selection.x = win_x;
    gs->selection.y = win_y;
    gs->selection.w = win_w;
    gs->selection.h = win_h;
}

void grid_cursorzoom(GridState *gs, int cursor_x, int cursor_y, int w, int h) {
    gs->selection.w = w;
    gs->selection.h = h;
    gs->selection.x = cursor_x - w / 2;
    gs->selection.y = cursor_y - h / 2;

    /* Clamp to current output bounds */
    OutputInfo *out = &gs->outputs[gs->current_output];
    if (gs->selection.x < out->x) gs->selection.x = out->x;
    if (gs->selection.y < out->y) gs->selection.y = out->y;
    if (gs->selection.x + w > out->x + out->w) gs->selection.x = out->x + out->w - w;
    if (gs->selection.y + h > out->y + out->h) gs->selection.y = out->y + out->h - h;
}

void grid_get_center(GridState *gs, int *cx, int *cy) {
    *cx = gs->selection.x + gs->selection.w / 2;
    *cy = gs->selection.y + gs->selection.h / 2;
}

void grid_free(GridState *gs) {
    if (gs->outputs) {
        for (int i = 0; i < gs->noutputs; i++) {
            free(gs->outputs[i].name);
        }
        free(gs->outputs);
    }
}