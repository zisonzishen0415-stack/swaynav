#include "commands.h"
#include "mouse.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
    const char *name;
    void (*func)(AppState *, const char *);
} CommandDispatch;

static CommandDispatch dispatch_table[] = {
    {"cut-up", cmd_cut_up},
    {"cut-down", cmd_cut_down},
    {"cut-left", cmd_cut_left},
    {"cut-right", cmd_cut_right},
    {"move-up", cmd_move_up},
    {"move-down", cmd_move_down},
    {"move-left", cmd_move_left},
    {"move-right", cmd_move_right},
    {"grid", cmd_grid},
    {"grid-nav", cmd_grid_nav},
    {"cell-select", cmd_cell_select},
    {"warp", cmd_warp},
    {"click", cmd_click},
    {"doubleclick", cmd_doubleclick},
    {"drag", cmd_drag},
    {"windowzoom", cmd_windowzoom},
    {"cursorzoom", cmd_cursorzoom},
    {"start", cmd_start},
    {"end", cmd_end},
    {"toggle-start", cmd_toggle},
    {"history-back", cmd_history_back},
    {"record", cmd_record},
    {"playback", cmd_playback},
    {"sh", cmd_shell},
    {"quit", cmd_quit},
    {NULL, NULL}
};

void cmd_execute(AppState *as, const char *command) {
    char *cmd = safe_strdup(command);
    char *args = cmd;
    while (isspace(*args)) args++;
    char *name = args;
    while (*args && !isspace(*args)) args++;
    if (*args) { *args = '\0'; args++; while (isspace(*args)) args++; }
    else args = "";

    for (int i = 0; dispatch_table[i].name; i++) {
        if (strcmp(name, dispatch_table[i].name) == 0) {
            dispatch_table[i].func(as, args);
            free(cmd);
            return;
        }
    }
    free(cmd);
}

void cmd_execute_sequence(AppState *as, const char *sequence) {
    if (!sequence) return;

    history_save(&as->history, &as->grid.selection);

    char *seq = safe_strdup(sequence);
    char *tok, *saveptr;

    tok = strtok_r(seq, ",", &saveptr);
    while (tok) {
        cmd_execute(as, tok);
        tok = strtok_r(NULL, ",", &saveptr);
    }

    free(seq);
}

void cmd_cut_up(AppState *as, const char *args) {
    grid_cut_up(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_cut_down(AppState *as, const char *args) {
    grid_cut_down(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_cut_left(AppState *as, const char *args) {
    grid_cut_left(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_cut_right(AppState *as, const char *args) {
    grid_cut_right(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_move_up(AppState *as, const char *args) {
    grid_move_up(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_move_down(AppState *as, const char *args) {
    grid_move_down(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_move_left(AppState *as, const char *args) {
    grid_move_left(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_move_right(AppState *as, const char *args) {
    grid_move_right(&as->grid, args);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}
void cmd_grid(AppState *as, const char *args) {
    int cols = 2, rows = 2;
    if (args && *args) sscanf(args, "%dx%d", &cols, &rows);
    grid_set(&as->grid, cols, rows);
}
void cmd_grid_nav(AppState *as, const char *args) {
    if (!args || !*args || strcmp(args, "toggle") == 0) grid_toggle_nav(&as->grid);
    else if (strcmp(args, "on") == 0) as->grid.grid_nav_active = 1;
    else if (strcmp(args, "off") == 0) as->grid.grid_nav_active = 0;
}
void cmd_cell_select(AppState *as, const char *args) { grid_cell_select(&as->grid, args); }

void cmd_warp(AppState *as, const char *args) {
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}

void cmd_click(AppState *as, const char *args) {
    int button = args && *args ? atoi(args) : 1;
    mouse_click(button);
}

void cmd_doubleclick(AppState *as, const char *args) {
    int button = args && *args ? atoi(args) : 1;
    mouse_doubleclick(button);
}

void cmd_drag(AppState *as, const char *args) {
    int button = 1;
    char mods[64] = "";
    if (args && *args) {
        char *argdup = safe_strdup(args);
        char *b = strtok(argdup, " ");
        char *m = strtok(NULL, "");
        if (b) button = atoi(b);
        if (m) strncpy(mods, trim(m), sizeof(mods) - 1);
        free(argdup);
    }
    if (as->dragging) { mouse_drag_end(as->drag_button); as->dragging = 0; }
    else {
        int cx, cy;
        grid_get_center(&as->grid, &cx, &cy);
        mouse_move(cx, cy);
        mouse_drag_start(button);
        as->dragging = 1;
        as->drag_button = button;
        strncpy(as->drag_modifiers, mods, sizeof(as->drag_modifiers) - 1);
    }
}

void cmd_windowzoom(AppState *as, const char *args) {
    SwayWindow *win = swayipc_get_focused_window();
    if (win) {
        grid_windowzoom(&as->grid, win->x, win->y, win->w, win->h);
        swayipc_free_window(win);
    }
}

void cmd_cursorzoom(AppState *as, const char *args) {
    int w = 300, h = 300;
    if (args && *args) sscanf(args, "%d %d", &w, &h);
    int cx, cy;
    if (swayipc_get_cursor_position(&cx, &cy) == 0) {
        grid_cursorzoom(&as->grid, cx, cy, w, h);
    }
}

void cmd_start(AppState *as, const char *args) {
    as->active = 1;
    grid_reset(&as->grid);
    history_clear(&as->history);
    history_save(&as->history, &as->grid.selection);
    int cx, cy;
    grid_get_center(&as->grid, &cx, &cy);
    mouse_move(cx, cy);
}

void cmd_end(AppState *as, const char *args) {
    as->active = 0;
    if (as->dragging) { mouse_drag_end(as->drag_button); as->dragging = 0; }
    if (as->recorder.is_recording) recorder_stop(&as->recorder);
}

void cmd_toggle(AppState *as, const char *args) {
    if (as->active) cmd_end(as, args);
    else cmd_start(as, args);
}

void cmd_history_back(AppState *as, const char *args) {
    history_back(&as->history, &as->grid.selection);
}

void cmd_record(AppState *as, const char *args) { }
void cmd_playback(AppState *as, const char *args) { }

void cmd_shell(AppState *as, const char *args) {
    if (!args || !*args) return;
    system(args);
}

void cmd_quit(AppState *as, const char *args) { gtk_main_quit(); }