#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default keybindings - vim style */
static const char *default_bindings[] = {
    "Escape end",
    "ctrl+[ end",

    "h cut-left",
    "j cut-down",
    "k cut-up",
    "l cut-right",
    "y cut-left,cut-up",
    "u cut-right,cut-up",
    "b cut-left,cut-down",
    "n cut-right,cut-down",

    "shift+h move-left",
    "shift+j move-down",
    "shift+k move-up",
    "shift+l move-right",
    "shift+y move-left,move-up",
    "shift+u move-right,move-up",
    "shift+b move-left,move-down",
    "shift+n move-right,move-down",

    "space warp,click 1,end",
    "Return warp,click 1,end",
    "semicolon end",
    "1 click 1",
    "2 click 2",
    "3 click 3",
    "4 click 4",
    "5 click 5",

    "a history-back",
    "e end",

    /* Arrow keys */
    "Left cut-left",
    "Down cut-down",
    "Up cut-up",
    "Right cut-right",
    "shift+Left move-left",
    "shift+Down move-down",
    "shift+Up move-up",
    "shift+Right move-right",

    /* Ctrl variants */
    "ctrl+h cut-left",
    "ctrl+j cut-down",
    "ctrl+k cut-up",
    "ctrl+l cut-right",
    "ctrl+y cut-left,cut-up",
    "ctrl+u cut-right,cut-up",
    "ctrl+b cut-left,cut-down",
    "ctrl+n cut-right,cut-down",

    NULL
};

void config_init(Config *cfg) {
    keybind_init(&cfg->keybinds);
    cfg->daemonize = 0;
    cfg->recordings_file = NULL;

    /* Load default bindings */
    for (int i = 0; default_bindings[i]; i++) {
        char *line = safe_strdup(default_bindings[i]);
        char *keyseq = strtok(line, " ");
        char *cmds = strtok(NULL, "");
        if (keyseq && cmds) {
            keybind_add(&cfg->keybinds, keyseq, cmds);
        }
        free(line);
    }
}

static int parse_line(Config *cfg, char *line) {
    /* Skip comments */
    char *comment = strchr(line, '#');
    if (comment) *comment = '\0';

    line = trim(line);
    if (line[0] == '\0') return 0;

    /* Special commands */
    if (strcmp(line, "clear") == 0) {
        keybind_clear(&cfg->keybinds);
        return 0;
    }

    if (strcmp(line, "daemonize") == 0) {
        cfg->daemonize = 1;
        return 0;
    }

    /* Keybinding */
    char *keyseq = strtok(line, " ");
    char *cmds = strtok(NULL, "");

    if (keyseq && cmds) {
        cmds = trim(cmds);
        keybind_add(&cfg->keybinds, keyseq, cmds);
    }

    return 0;
}

int config_load(Config *cfg, const char *path) {
    char *expanded = expand_path(path);
    if (!expanded) return -1;

    FILE *fp = fopen(expanded, "r");
    free(expanded);

    if (!fp) return -1;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove newline */
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        nl = strchr(line, '\r');
        if (nl) *nl = '\0';

        parse_line(cfg, line);
    }

    fclose(fp);
    return 0;
}

int config_load_default(Config *cfg) {
    /* Try XDG config first */
    const char *xdg = getenv("XDG_CONFIG_HOME");
    char path[256];

    if (xdg) {
        snprintf(path, sizeof(path), "%s/swaynav/config", xdg);
        if (config_load(cfg, path) == 0) return 0;
    }

    /* Try ~/.config/swaynav/config */
    snprintf(path, sizeof(path), "~/.config/swaynav/config");
    if (config_load(cfg, path) == 0) return 0;

    /* Try ~/.swaynavrc */
    if (config_load(cfg, "~/.swaynavrc") == 0) return 0;

    /* Try global config */
    if (config_load(cfg, GLOBAL_CONFIG) == 0) return 0;

    return -1;
}

void config_free(Config *cfg) {
    keybind_free(&cfg->keybinds);
    free(cfg->recordings_file);
}