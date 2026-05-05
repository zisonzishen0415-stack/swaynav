#include "swayipc.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <gtk/gtk.h>

/* Generic implementation using GDK - works on any Wayland compositor */

SwayOutput *swayipc_get_outputs(int *count) {
    GdkDisplay *display = gdk_display_get_default();
    if (!display) {
        fprintf(stderr, "swaynav: cannot get GDK display\n");
        return NULL;
    }

    int n = gdk_display_get_n_monitors(display);
    if (n <= 0) {
        fprintf(stderr, "swaynav: no monitors found\n");
        return NULL;
    }

    SwayOutput *outputs = calloc(n, sizeof(SwayOutput));
    if (!outputs) return NULL;

    for (int i = 0; i < n; i++) {
        GdkMonitor *monitor = gdk_display_get_monitor(display, i);
        GdkRectangle rect;
        gdk_monitor_get_geometry(monitor, &rect);

        outputs[i].x = rect.x;
        outputs[i].y = rect.y;
        outputs[i].w = rect.width;
        outputs[i].h = rect.height;
        outputs[i].name = safe_strdup(gdk_monitor_get_model(monitor) ?: "unknown");
        outputs[i].active = gdk_monitor_is_primary(monitor);
    }

    *count = n;
    return outputs;
}

static SwayWindow *find_focused_window(json_object *node) {
    if (!node) return NULL;

    json_object *focused;
    if (json_object_object_get_ex(node, "focused", &focused)) {
        if (json_object_get_boolean(focused)) {
            json_object *type;
            if (json_object_object_get_ex(node, "type", &type)) {
                const char *tstr = json_object_get_string(type);
                if (strcmp(tstr, "con") == 0 || strcmp(tstr, "floating_con") == 0) {
                    SwayWindow *win = calloc(1, sizeof(SwayWindow));

                    json_object *rect, *name;
                    if (json_object_object_get_ex(node, "rect", &rect)) {
                        json_object *x, *y, *w, *h;
                        json_object_object_get_ex(rect, "x", &x);
                        json_object_object_get_ex(rect, "y", &y);
                        json_object_object_get_ex(rect, "width", &w);
                        json_object_object_get_ex(rect, "height", &h);
                        win->x = json_object_get_int(x);
                        win->y = json_object_get_int(y);
                        win->w = json_object_get_int(w);
                        win->h = json_object_get_int(h);
                    }

                    if (json_object_object_get_ex(node, "name", &name)) {
                        win->name = safe_strdup(json_object_get_string(name));
                    }

                    win->focused = 1;
                    return win;
                }
            }
        }
    }

    /* Check children */
    json_object *nodes, *floating_nodes;
    if (json_object_object_get_ex(node, "nodes", &nodes)) {
        int n = json_object_array_length(nodes);
        for (int i = 0; i < n; i++) {
            SwayWindow *win = find_focused_window(json_object_array_get_idx(nodes, i));
            if (win) return win;
        }
    }

    if (json_object_object_get_ex(node, "floating_nodes", &floating_nodes)) {
        int n = json_object_array_length(floating_nodes);
        for (int i = 0; i < n; i++) {
            SwayWindow *win = find_focused_window(json_object_array_get_idx(floating_nodes, i));
            if (win) return win;
        }
    }

    return NULL;
}

SwayWindow *swayipc_get_focused_window(void) {
    FILE *p = popen("swaymsg -t get_tree -r", "r");
    if (!p) return NULL;

    char *buf = NULL;
    size_t total = 0;
    char chunk[4096];

    while (fgets(chunk, sizeof(chunk), p)) {
        size_t len = strlen(chunk);
        char *new_buf = realloc(buf, total + len + 1);
        if (!new_buf) {
            free(buf);
            pclose(p);
            return NULL;
        }
        buf = new_buf;
        memcpy(buf + total, chunk, len);
        total += len;
        buf[total] = '\0';
    }
    pclose(p);

    if (!buf) return NULL;

    json_object *resp = json_tokener_parse(buf);
    free(buf);

    if (!resp) return NULL;

    SwayWindow *win = find_focused_window(resp);
    json_object_put(resp);

    return win;
}

int swayipc_get_cursor_position(int *x, int *y) {
    FILE *p = popen("swaymsg -t get_seats -r", "r");
    if (!p) return -1;

    char *buf = NULL;
    size_t total = 0;
    char chunk[4096];

    while (fgets(chunk, sizeof(chunk), p)) {
        size_t len = strlen(chunk);
        char *new_buf = realloc(buf, total + len + 1);
        if (!new_buf) {
            free(buf);
            pclose(p);
            return -1;
        }
        buf = new_buf;
        memcpy(buf + total, chunk, len);
        total += len;
        buf[total] = '\0';
    }
    pclose(p);

    if (!buf) return -1;

    json_object *resp = json_tokener_parse(buf);
    free(buf);

    if (!resp || !json_object_is_type(resp, json_type_array)) {
        if (resp) json_object_put(resp);
        return -1;
    }

    /* Find seat0 */
    for (int i = 0; i < (int)json_object_array_length(resp); i++) {
        json_object *seat = json_object_array_get_idx(resp, i);
        json_object *name, *cursor;
        if (json_object_object_get_ex(seat, "name", &name)) {
            if (strcmp(json_object_get_string(name), "seat0") == 0) {
                if (json_object_object_get_ex(seat, "cursor", &cursor)) {
                    json_object *cx, *cy;
                    if (json_object_object_get_ex(cursor, "x", &cx) &&
                        json_object_object_get_ex(cursor, "y", &cy)) {
                        *x = json_object_get_int(cx);
                        *y = json_object_get_int(cy);
                        json_object_put(resp);
                        return 0;
                    }
                }
            }
        }
    }

    json_object_put(resp);
    return -1;
}

void swayipc_free_outputs(SwayOutput *outputs, int count) {
    if (!outputs) return;
    for (int i = 0; i < count; i++) {
        free(outputs[i].name);
    }
    free(outputs);
}

void swayipc_free_window(SwayWindow *win) {
    if (!win) return;
    free(win->name);
    free(win);
}

/* Check if running under Sway compositor */
int swayipc_is_sway(void) {
    /* Check SWAYSOCK environment variable */
    if (getenv("SWAYSOCK")) return 1;

    /* Try to run swaymsg, if it works we're under Sway */
    FILE *p = popen("swaymsg -t get_version -r 2>/dev/null", "r");
    if (!p) return 0;

    char buf[256];
    if (fgets(buf, sizeof(buf), p)) {
        pclose(p);
        return 1;  /* Got response, likely Sway */
    }
    pclose(p);
    return 0;
}

/* Get ydotoold device pointer_accel value */
float swayipc_get_ydotoold_accel(void) {
    if (!swayipc_is_sway()) return -999;  /* Not Sway, skip */

    FILE *p = popen("swaymsg -t get_inputs -r", "r");
    if (!p) return -999;

    char *buf = NULL;
    size_t total = 0;
    char chunk[4096];

    while (fgets(chunk, sizeof(chunk), p)) {
        size_t len = strlen(chunk);
        char *new_buf = realloc(buf, total + len + 1);
        if (!new_buf) {
            free(buf);
            pclose(p);
            return -999;
        }
        buf = new_buf;
        memcpy(buf + total, chunk, len);
        total += len;
        buf[total] = '\0';
    }
    pclose(p);

    if (!buf) return -999;

    json_object *resp = json_tokener_parse(buf);
    free(buf);

    if (!resp || !json_object_is_type(resp, json_type_array)) {
        if (resp) json_object_put(resp);
        return -999;
    }

    /* Find ydotoold device */
    for (int i = 0; i < (int)json_object_array_length(resp); i++) {
        json_object *dev = json_object_array_get_idx(resp, i);
        json_object *identifier, *libinput;

        if (json_object_object_get_ex(dev, "identifier", &identifier)) {
            const char *id = json_object_get_string(identifier);
            if (strstr(id, "ydotoold")) {
                if (json_object_object_get_ex(dev, "libinput", &libinput)) {
                    json_object *accel;
                    if (json_object_object_get_ex(libinput, "accel_speed", &accel)) {
                        float val = (float)json_object_get_double(accel);
                        json_object_put(resp);
                        return val;
                    }
                }
            }
        }
    }

    json_object_put(resp);
    return -999;  /* ydotoold device not found */
}

/* Set ydotoold device pointer_accel to 0 */
int swayipc_set_ydotoold_accel_zero(void) {
    FILE *p = popen("swaymsg input 9011:26214:ydotoold_virtual_device pointer_accel 0", "r");
    if (!p) return -1;

    char buf[256];
    int success = 0;

    /* Check for success response */
    while (fgets(buf, sizeof(buf), p)) {
        if (strstr(buf, "true") || strstr(buf, "success")) {
            success = 1;
        }
    }
    pclose(p);
    return success ? 0 : -1;
}