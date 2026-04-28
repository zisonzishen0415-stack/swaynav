#ifndef SWAYNAV_COMMANDS_H
#define SWAYNAV_COMMANDS_H

#include "grid.h"
#include "config.h"
#include "history.h"
#include "recorder.h"
#include "swayipc.h"

/* App state */
typedef struct {
    int active;
    int dragging;
    int drag_button;
    char drag_modifiers[64];

    GridState grid;
    History history;
    Config config;
    RecorderState recorder;

    /* For grid-nav mode */
    int grid_nav_col;
    int grid_nav_row;
    int grid_nav_state;  /* 0=column, 1=row */
} AppState;

/* Execute a single command */
void cmd_execute(AppState *as, const char *command);

/* Execute comma-separated command sequence */
void cmd_execute_sequence(AppState *as, const char *sequence);

/* Command implementations */
void cmd_cut_up(AppState *as, const char *args);
void cmd_cut_down(AppState *as, const char *args);
void cmd_cut_left(AppState *as, const char *args);
void cmd_cut_right(AppState *as, const char *args);
void cmd_move_up(AppState *as, const char *args);
void cmd_move_down(AppState *as, const char *args);
void cmd_move_left(AppState *as, const char *args);
void cmd_move_right(AppState *as, const char *args);

void cmd_grid(AppState *as, const char *args);
void cmd_grid_nav(AppState *as, const char *args);
void cmd_cell_select(AppState *as, const char *args);

void cmd_warp(AppState *as, const char *args);
void cmd_click(AppState *as, const char *args);
void cmd_doubleclick(AppState *as, const char *args);
void cmd_drag(AppState *as, const char *args);

void cmd_windowzoom(AppState *as, const char *args);
void cmd_cursorzoom(AppState *as, const char *args);

void cmd_start(AppState *as, const char *args);
void cmd_end(AppState *as, const char *args);
void cmd_toggle(AppState *as, const char *args);
void cmd_history_back(AppState *as, const char *args);
void cmd_record(AppState *as, const char *args);
void cmd_playback(AppState *as, const char *args);
void cmd_shell(AppState *as, const char *args);
void cmd_quit(AppState *as, const char *args);

#endif /* SWAYNAV_COMMANDS_H */