#include <gtk/gtk.h>
#include "overlay.h"
#include "grid.h"
#include "config.h"
#include "commands.h"
#include "swayipc.h"
#include "mouse.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#define SOCKET_PATH "/tmp/swaynav.sock"
#define MAX_DISPLAY_WAIT 30  /* seconds to wait for display */

static AppState app_state;
static GtkWidget *overlay_window = NULL;
static int daemon_sock = -1;

static void show_overlay(void) {
    if (!app_state.active) {
        app_state.active = 1;
        grid_reset(&app_state.grid);
        history_clear(&app_state.history);
        history_save(&app_state.history, &app_state.grid.selection);
        gtk_widget_show_all(overlay_window);
        gtk_window_present(GTK_WINDOW(overlay_window));
    }
}

static void hide_overlay(void) {
    if (app_state.active) {
        app_state.active = 0;
        if (app_state.dragging) {
            mouse_drag_end(app_state.drag_button);
            app_state.dragging = 0;
        }
        gtk_widget_queue_draw(overlay_window);
        gtk_widget_hide(overlay_window);
    }
}

static void handle_command(const char *cmd) {
    if (strcmp(cmd, "start") == 0) {
        show_overlay();
    } else if (strcmp(cmd, "end") == 0) {
        hide_overlay();
    } else if (strcmp(cmd, "toggle") == 0) {
        if (app_state.active) hide_overlay();
        else show_overlay();
    } else if (strcmp(cmd, "quit") == 0) {
        gtk_main_quit();
    } else {
        cmd_execute_sequence(&app_state, cmd);
        gtk_widget_queue_draw(overlay_window);
        if (!app_state.active) gtk_widget_hide(overlay_window);
    }
}

static gboolean on_socket_data(GIOChannel *channel, GIOCondition condition, gpointer data) {
    if (condition & G_IO_IN) {
        int client = accept(daemon_sock, NULL, NULL);
        if (client >= 0) {
            char buf[256];
            ssize_t n = read(client, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                handle_command(buf);
            }
            close(client);
        }
    }
    return TRUE;
}

static int setup_socket(void) {
    /* Remove old socket */
    unlink(SOCKET_PATH);

    daemon_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (daemon_sock < 0) {
        fprintf(stderr, "swaynav: socket creation failed\n");
        return -1;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(daemon_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "swaynav: socket bind failed\n");
        close(daemon_sock);
        return -1;
    }

    if (listen(daemon_sock, 5) < 0) {
        fprintf(stderr, "swaynav: socket listen failed\n");
        close(daemon_sock);
        return -1;
    }

    /* Add to GTK main loop */
    GIOChannel *channel = g_io_channel_unix_new(daemon_sock);
    g_io_add_watch(channel, G_IO_IN, on_socket_data, NULL);

    fprintf(stderr, "swaynav: listening on %s\n", SOCKET_PATH);
    return 0;
}

static int send_to_daemon(const char *cmd) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    write(sock, cmd, strlen(cmd));
    close(sock);
    return 0;
}

static void sighandler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        unlink(SOCKET_PATH);
        gtk_main_quit();
    }
}

static void init_outputs(void) {
    int count;
    SwayOutput *outputs = swayipc_get_outputs(&count);

    if (!outputs || count == 0) {
        fprintf(stderr, "swaynav: cannot get outputs from Sway\n");
        exit(1);
    }

    fprintf(stderr, "swaynav: found %d outputs\n", count);
    for (int i = 0; i < count; i++) {
        fprintf(stderr, "  output %d: %s @ %d,%d size %dx%d\n",
                i, outputs[i].name ? outputs[i].name : "unknown",
                outputs[i].x, outputs[i].y, outputs[i].w, outputs[i].h);
    }

    OutputInfo *outinfos = calloc(count, sizeof(OutputInfo));
    for (int i = 0; i < count; i++) {
        outinfos[i].x = outputs[i].x;
        outinfos[i].y = outputs[i].y;
        outinfos[i].w = outputs[i].w;
        outinfos[i].h = outputs[i].h;
        outinfos[i].name = outputs[i].name ? safe_strdup(outputs[i].name) : NULL;
    }

    grid_init(&app_state.grid, outinfos, count);
    swayipc_free_outputs(outputs, count);
}

static void print_usage(void) {
    printf("swaynav - keyboard-driven mouse control for Sway/Wayland\n\n");
    printf("Usage: swaynav [command]\n\n");
    printf("Commands:\n");
    printf("  (no args)  Start as background daemon\n");
    printf("  start      Show overlay\n");
    printf("  end        Hide overlay\n");
    printf("  toggle     Toggle overlay\n");
    printf("  quit       Quit daemon\n\n");
    printf("Add to your Wayland compositor config:\n");
    printf("  Sway: exec swaynav\n");
    printf("  Hyprland: exec-once = swaynav\n");
    printf("  River: riverctl spawn swaynav\n\n");
}

/* Wait for Wayland display to be available */
static int wait_for_display(void) {
    const char *wayland_display = getenv("WAYLAND_DISPLAY");
    const char *xdisplay = getenv("DISPLAY");

    if (wayland_display || xdisplay) {
        return 0;  /* Already available */
    }

    fprintf(stderr, "swaynav: waiting for display (up to %d seconds)...\n", MAX_DISPLAY_WAIT);

    for (int i = 0; i < MAX_DISPLAY_WAIT; i++) {
        sleep(1);
        wayland_display = getenv("WAYLAND_DISPLAY");
        xdisplay = getenv("DISPLAY");
        if (wayland_display || xdisplay) {
            fprintf(stderr, "swaynav: display available after %d seconds\n", i + 1);
            return 0;
        }
    }

    fprintf(stderr, "swaynav: timeout waiting for display\n");
    return -1;
}

int main(int argc, char **argv) {
    /* Handle help */
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage();
        return 0;
    }

    /* Try to send command to existing daemon */
    if (argc > 1 && strcmp(argv[1], "--daemon") != 0) {
        if (send_to_daemon(argv[1]) == 0) {
            fprintf(stderr, "swaynav: sent '%s' to daemon\n", argv[1]);
            return 0;
        }
        fprintf(stderr, "swaynav: no daemon running, starting...\n");
    }

    /* Start daemon */
    fprintf(stderr, "swaynav: starting daemon...\n");

    /* Wait for display to be available (for systemd service startup) */
    if (wait_for_display() < 0) {
        fprintf(stderr, "swaynav: no display available, exiting\n");
        return 1;
    }

    /* Use gtk_init_check to avoid abort on failure */
    if (!gtk_init_check(&argc, &argv)) {
        fprintf(stderr, "swaynav: gtk_init failed\n");
        return 1;
    }

    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

    /* Initialize */
    config_init(&app_state.config);
    config_load_default(&app_state.config);
    fprintf(stderr, "swaynav: %d keybindings loaded\n", app_state.config.keybinds.count);

    history_init(&app_state.history);
    recorder_init(&app_state.recorder, "~/.config/swaynav/recordings");
    app_state.active = 0;

    init_outputs();

    overlay_window = overlay_create(&app_state);

    /* Setup socket for commands */
    if (setup_socket() < 0) {
        fprintf(stderr, "swaynav: failed to setup socket\n");
        return 1;
    }

    /* Handle initial command if provided */
    if (argc > 1 && strcmp(argv[1], "start") == 0) {
        show_overlay();
    }

    /* Run main loop */
    fprintf(stderr, "swaynav: daemon ready\n");
    gtk_main();

    /* Cleanup */
    unlink(SOCKET_PATH);
    config_free(&app_state.config);
    grid_free(&app_state.grid);
    recorder_free(&app_state.recorder);

    fprintf(stderr, "swaynav: exited\n");
    return 0;
}