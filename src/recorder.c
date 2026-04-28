#include "recorder.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recorder_init(RecorderState *rs, const char *filename) {
    memset(rs, 0, sizeof(RecorderState));
    rs->active_slot = -1;
    rs->is_recording = 0;
    rs->is_playback_mode = 0;

    if (filename) {
        rs->filename = expand_path(filename);
        recorder_load(rs);
    }
}

void recorder_start(RecorderState *rs, guint keyval, GdkModifierType mods) {
    /* Find empty slot based on key */
    int slot = -1;

    /* Use lowercase letter as slot index */
    if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) {
        slot = keyval - GDK_KEY_a;
    } else if (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z) {
        slot = keyval - GDK_KEY_A;
    }

    if (slot < 0 || slot >= RECORDER_MAX_SLOTS) {
        fprintf(stderr, "swaynav: invalid recording slot key\n");
        return;
    }

    /* Clear existing recording in this slot */
    if (rs->slots[slot].commands) {
        free(rs->slots[slot].commands);
        rs->slots[slot].commands = NULL;
        rs->slots[slot].count = 0;
    }

    rs->active_slot = slot;
    rs->is_recording = 1;
    rs->slots[slot].keyval = keyval;
    rs->slots[slot].mods = mods;
}

void recorder_stop(RecorderState *rs) {
    if (rs->is_recording && rs->active_slot >= 0) {
        recorder_save(rs);
    }

    rs->is_recording = 0;
    rs->active_slot = -1;
}

void recorder_add_key(RecorderState *rs, guint keyval, GdkModifierType mods) {
    if (!rs->is_recording || rs->active_slot < 0) return;

    Recording *rec = &rs->slots[rs->active_slot];

    /* Append key sequence as command string */
    char keystr[64];
    snprintf(keystr, sizeof(keystr), "%u:%u,", keyval, mods);

    if (rec->commands) {
        size_t len = strlen(rec->commands) + strlen(keystr) + 1;
        char *new_cmds = realloc(rec->commands, len);
        if (new_cmds) {
            rec->commands = new_cmds;
            strcat(rec->commands, keystr);
        }
    } else {
        rec->commands = safe_strdup(keystr);
    }

    rec->count++;
}

Recording *recorder_find(RecorderState *rs, guint keyval, GdkModifierType mods) {
    /* Find recording by key */
    int slot = -1;

    if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) {
        slot = keyval - GDK_KEY_a;
    } else if (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z) {
        slot = keyval - GDK_KEY_A;
    }

    if (slot >= 0 && slot < RECORDER_MAX_SLOTS) {
        if (rs->slots[slot].commands) {
            return &rs->slots[slot];
        }
    }

    return NULL;
}

void recorder_playback(RecorderState *rs) {
    rs->is_playback_mode = 1;
}

void recorder_save(RecorderState *rs) {
    if (!rs->filename) return;

    FILE *fp = fopen(rs->filename, "w");
    if (!fp) {
        fprintf(stderr, "swaynav: cannot save recordings to %s\n", rs->filename);
        return;
    }

    for (int i = 0; i < RECORDER_MAX_SLOTS; i++) {
        if (rs->slots[i].commands) {
            fprintf(fp, "%d:%u:%u:%s\n", i, rs->slots[i].keyval, rs->slots[i].mods, rs->slots[i].commands);
        }
    }

    fclose(fp);
}

void recorder_load(RecorderState *rs) {
    if (!rs->filename) return;

    FILE *fp = fopen(rs->filename, "r");
    if (!fp) return;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        int slot;
        guint keyval, mods;
        char *cmds;

        if (sscanf(line, "%d:%u:%u:", &slot, &keyval, &mods) == 3) {
            cmds = strchr(line, ':');
            if (cmds) {
                cmds = strchr(cmds + 1, ':');
                if (cmds) {
                    cmds = strchr(cmds + 1, ':');
                    if (cmds) {
                        cmds++;
                        char *nl = strchr(cmds, '\n');
                        if (nl) *nl = '\0';
                        rs->slots[slot].keyval = keyval;
                        rs->slots[slot].mods = mods;
                        rs->slots[slot].commands = safe_strdup(cmds);
                    }
                }
            }
        }
    }

    fclose(fp);
}

void recorder_free(RecorderState *rs) {
    for (int i = 0; i < RECORDER_MAX_SLOTS; i++) {
        free(rs->slots[i].commands);
    }
    free(rs->filename);
}