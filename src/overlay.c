#include "overlay.h"
#include "grid.h"
#include "keybind.h"
#include "mouse.h"
#include "swayipc.h"
#include <gtk-layer-shell.h>
#include <cairo.h>
#include <string.h>

static AppState *g_state = NULL;
static GtkWidget *g_window = NULL;

static void draw_grid(cairo_t *cr, AppState *as) {
    GridRect *sel = &as->grid.selection;
    OutputInfo *out = &as->grid.outputs[as->grid.current_output];

    double sx = sel->x - out->x;
    double sy = sel->y - out->y;
    double sw = sel->w;
    double sh = sel->h;

    /* Draw keynav-style crosshair lines */
    cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 2.0);

    /* Horizontal line through center */
    double cy = sy + sh / 2;
    cairo_move_to(cr, 0, cy);
    cairo_line_to(cr, out->w, cy);
    cairo_stroke(cr);

    /* Vertical line through center */
    double cx = sx + sw / 2;
    cairo_move_to(cr, cx, 0);
    cairo_line_to(cr, cx, out->h);
    cairo_stroke(cr);

    /* Selection rectangle border */
    cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 3.0);
    cairo_rectangle(cr, sx, sy, sw, sh);
    cairo_stroke(cr);

    /* Corner markers */
    cairo_set_line_width(cr, 2.0);
    int marker_len = 15;

    cairo_move_to(cr, sx, sy + marker_len);
    cairo_line_to(cr, sx, sy);
    cairo_line_to(cr, sx + marker_len, sy);
    cairo_stroke(cr);

    cairo_move_to(cr, sx + sw - marker_len, sy);
    cairo_line_to(cr, sx + sw, sy);
    cairo_line_to(cr, sx + sw, sy + marker_len);
    cairo_stroke(cr);

    cairo_move_to(cr, sx, sy + sh - marker_len);
    cairo_line_to(cr, sx, sy + sh);
    cairo_line_to(cr, sx + marker_len, sy + sh);
    cairo_stroke(cr);

    cairo_move_to(cr, sx + sw - marker_len, sy + sh);
    cairo_line_to(cr, sx + sw, sy + sh);
    cairo_line_to(cr, sx + sw, sy + sh - marker_len);
    cairo_stroke(cr);

    /* Grid lines */
    if (as->grid.grid_cols > 1 || as->grid.grid_rows > 1) {
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.5);
        cairo_set_line_width(cr, 1.0);

        int cell_w = sw / as->grid.grid_cols;
        int cell_h = sh / as->grid.grid_rows;

        for (int i = 1; i < as->grid.grid_cols; i++) {
            double x = sx + i * cell_w;
            cairo_move_to(cr, x, sy);
            cairo_line_to(cr, x, sy + sh);
            cairo_stroke(cr);
        }

        for (int i = 1; i < as->grid.grid_rows; i++) {
            double y = sy + i * cell_h;
            cairo_move_to(cr, sx, y);
            cairo_line_to(cr, sx + sw, y);
            cairo_stroke(cr);
        }
    }

    /* Grid nav labels */
    if (as->grid.grid_nav_active) {
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 16);

        char label[3];
        label[2] = '\0';

        int cell_w = sw / as->grid.grid_cols;
        int cell_h = sh / as->grid.grid_rows;

        for (int col = 0; col < as->grid.grid_cols; col++) {
            label[0] = 'A' + col;
            for (int row = 0; row < as->grid.grid_rows; row++) {
                label[1] = 'A' + row;

                double lcx = sx + col * cell_w + cell_w / 2;
                double lcy = sy + row * cell_h + cell_h / 2;

                cairo_text_extents_t extents;
                cairo_text_extents(cr, label, &extents);

                cairo_set_source_rgba(cr, 0.2, 0.2, 0.3, 0.8);
                cairo_rectangle(cr, lcx - extents.width/2 - 5, lcy - extents.height/2 - 3, extents.width + 10, extents.height + 6);
                cairo_fill(cr);

                cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_move_to(cr, lcx - extents.width/2, lcy + extents.height/2);
                cairo_show_text(cr, label);
            }
        }
    }
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppState *as = (AppState *)data;

    /* Always clear first */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    if (!as->active) return FALSE;

    draw_grid(cr, as);
    return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    AppState *as = (AppState *)data;
    guint keyval = event->keyval;
    GdkModifierType mods = event->state;

    if (as->recorder.is_recording) {
        KeyBinding *kb = keybind_find(&as->config.keybinds, keyval, mods);
        if (kb && strncmp(kb->commands, "record", 6) == 0) {
            recorder_stop(&as->recorder);
            overlay_redraw(widget, as);
            return TRUE;
        }
        recorder_add_key(&as->recorder, keyval, mods);
        return TRUE;
    }

    if (as->recorder.is_playback_mode) {
        Recording *rec = recorder_find(&as->recorder, keyval, mods);
        if (rec) {
            as->recorder.is_playback_mode = 0;
            return TRUE;
        }
        as->recorder.is_playback_mode = 0;
        return TRUE;
    }

    KeyBinding *kb = keybind_find(&as->config.keybinds, keyval, mods);
    if (kb) {
        if (strncmp(kb->commands, "record", 6) == 0) {
            as->recorder.is_recording = 1;
            overlay_redraw(widget, as);
            return TRUE;
        }

        if (strncmp(kb->commands, "playback", 8) == 0) {
            as->recorder.is_playback_mode = 1;
            overlay_redraw(widget, as);
            return TRUE;
        }

        cmd_execute_sequence(as, kb->commands);
        overlay_redraw(widget, as);

        if (!as->active) {
            gtk_widget_hide(widget);
        }

        return TRUE;
    }

    return FALSE;
}

GtkWidget *overlay_create(AppState *as) {
    g_state = as;

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_window = window;

    GtkWindow *gtkwin = GTK_WINDOW(window);

    /* Layer shell setup */
    gtk_layer_init_for_window(gtkwin);
    gtk_layer_set_layer(gtkwin, GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_exclusive_zone(gtkwin, -1);

    gtk_layer_set_anchor(gtkwin, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(gtkwin, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor(gtkwin, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(gtkwin, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    /* Exclusive keyboard mode */
    gtk_layer_set_keyboard_mode(gtkwin, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

    /* Connect signals */
    g_signal_connect(window, "draw", G_CALLBACK(on_draw), as);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), as);

    /* Transparent window */
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual) {
        gtk_widget_set_visual(window, visual);
    }

    gtk_window_set_decorated(gtkwin, FALSE);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_window_set_accept_focus(gtkwin, TRUE);
    gtk_window_set_focus_on_map(gtkwin, TRUE);

    /* Enable key events */
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    return window;
}

void overlay_show(GtkWidget *overlay) {
    fprintf(stderr, "swaynav: overlay_show called\n");
    gtk_widget_show_all(overlay);
    gtk_window_present(GTK_WINDOW(overlay));
    fprintf(stderr, "swaynav: overlay shown\n");
}

void overlay_hide(GtkWidget *overlay) {
    fprintf(stderr, "swaynav: overlay_hide called\n");
    gtk_widget_hide(overlay);
}

void overlay_redraw(GtkWidget *overlay, AppState *as) {
    (void)as;
    gtk_widget_queue_draw(overlay);
}

int overlay_get_current_output(GtkWidget *overlay) {
    GdkDisplay *display = gdk_display_get_default();
    GdkWindow *gdkwin = gtk_widget_get_window(overlay);
    if (!gdkwin) return 0;

    GdkMonitor *monitor = gdk_display_get_monitor_at_window(display, gdkwin);

    GdkRectangle rect;
    gdk_monitor_get_geometry(monitor, &rect);

    for (int i = 0; i < g_state->grid.noutputs; i++) {
        if (g_state->grid.outputs[i].x == rect.x && g_state->grid.outputs[i].y == rect.y) {
            return i;
        }
    }

    return 0;
}