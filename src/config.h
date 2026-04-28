#ifndef SWAYNAV_CONFIG_H
#define SWAYNAV_CONFIG_H

#include "keybind.h"

/* Global config paths */
#define CONFIG_FILE "~/.config/swaynav/config"
#define GLOBAL_CONFIG "/etc/swaynav/config"

typedef struct {
    KeyBindState keybinds;
    int daemonize;
    char *recordings_file;
} Config;

/* Initialize config with defaults */
void config_init(Config *cfg);

/* Load config file */
int config_load(Config *cfg, const char *path);

/* Load default config */
int config_load_default(Config *cfg);

/* Free config resources */
void config_free(Config *cfg);

#endif /* SWAYNAV_CONFIG_H */