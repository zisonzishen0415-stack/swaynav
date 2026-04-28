#ifndef SWAYNAV_RECORDER_H
#define SWAYNAV_RECORDER_H

#include "keybind.h"

#define RECORDER_MAX_SLOTS 26  /* a-z */

typedef struct {
    guint keyval;
    GdkModifierType mods;
    char *commands;
    int count;
} Recording;

typedef struct {
    Recording slots[RECORDER_MAX_SLOTS];
    int active_slot;
    int is_recording;
    int is_playback_mode;
    char *filename;
} RecorderState;

void recorder_init(RecorderState *rs, const char *filename);
void recorder_start(RecorderState *rs, guint keyval, GdkModifierType mods);
void recorder_stop(RecorderState *rs);
void recorder_add_key(RecorderState *rs, guint keyval, GdkModifierType mods);
Recording *recorder_find(RecorderState *rs, guint keyval, GdkModifierType mods);
void recorder_playback(RecorderState *rs);
void recorder_save(RecorderState *rs);
void recorder_load(RecorderState *rs);
void recorder_free(RecorderState *rs);

#endif /* SWAYNAV_RECORDER_H */