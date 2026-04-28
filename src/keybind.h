#ifndef SWAYNAV_KEYBIND_H
#define SWAYNAV_KEYBIND_H

#include <gtk/gtk.h>

/* Keybinding structure */
typedef struct {
    guint keyval;
    GdkModifierType mods;
    char *commands;
} KeyBinding;

/* Keybinding list */
typedef struct {
    KeyBinding *bindings;
    int count;
    int capacity;
} KeyBindState;

/* Initialize keybinding state */
void keybind_init(KeyBindState *ks);

/* Parse keybinding string and add */
int keybind_add(KeyBindState *ks, const char *keyseq, const char *commands);

/* Clear all bindings */
void keybind_clear(KeyBindState *ks);

/* Find binding for event */
KeyBinding *keybind_find(KeyBindState *ks, guint keyval, GdkModifierType mods);

/* Free resources */
void keybind_free(KeyBindState *ks);

#endif /* SWAYNAV_KEYBIND_H */