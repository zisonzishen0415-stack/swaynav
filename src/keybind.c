#include "keybind.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void keybind_init(KeyBindState *ks) {
    ks->bindings = NULL;
    ks->count = 0;
    ks->capacity = 0;
}

static guint parse_keyval(const char *name) {
    /* Handle common key names */
    if (strcmp(name, "space") == 0) return GDK_KEY_space;
    if (strcmp(name, "Return") == 0) return GDK_KEY_Return;
    if (strcmp(name, "Escape") == 0 || strcmp(name, "Esc") == 0) return GDK_KEY_Escape;
    if (strcmp(name, "semicolon") == 0) return GDK_KEY_semicolon;
    if (strcmp(name, "bracketleft") == 0 || strcmp(name, "[") == 0) return GDK_KEY_bracketleft;
    if (strcmp(name, "bracketright") == 0 || strcmp(name, "]") == 0) return GDK_KEY_bracketright;
    if (strcmp(name, "at") == 0 || strcmp(name, "@") == 0) return GDK_KEY_at;
    if (strcmp(name, "Left") == 0) return GDK_KEY_Left;
    if (strcmp(name, "Right") == 0) return GDK_KEY_Right;
    if (strcmp(name, "Up") == 0) return GDK_KEY_Up;
    if (strcmp(name, "Down") == 0) return GDK_KEY_Down;
    if (strcmp(name, "Insert") == 0) return GDK_KEY_Insert;
    if (strcmp(name, "Delete") == 0) return GDK_KEY_Delete;
    if (strcmp(name, "Home") == 0) return GDK_KEY_Home;
    if (strcmp(name, "End") == 0) return GDK_KEY_End;
    if (strcmp(name, "Prior") == 0) return GDK_KEY_Page_Up;
    if (strcmp(name, "Next") == 0) return GDK_KEY_Page_Down;

    /* Single character keys */
    if (strlen(name) == 1) {
        char c = name[0];
        if (isalpha(c)) {
            return GDK_KEY_a + (tolower(c) - 'a');
        } else if (isdigit(c)) {
            return GDK_KEY_0 + (c - '0');
        } else if (c == '@') {
            return GDK_KEY_at;
        }
    }

    /* Try GDK key name */
    return gdk_keyval_from_name(name);
}

static GdkModifierType parse_mods(const char *keyseq) {
    GdkModifierType mods = 0;

    char *dup = safe_strdup(keyseq);
    char *tok, *saveptr;

    tok = strtok_r(dup, "+", &saveptr);
    while (tok) {
        /* Check current token for modifiers, skip last token (the key) */
        char *next = strtok_r(NULL, "+", &saveptr);
        if (next == NULL) break; /* Last token is the key */

        if (strcasecmp(tok, "shift") == 0) mods |= GDK_SHIFT_MASK;
        else if (strcasecmp(tok, "ctrl") == 0 || strcasecmp(tok, "control") == 0) mods |= GDK_CONTROL_MASK;
        else if (strcasecmp(tok, "alt") == 0 || strcasecmp(tok, "mod1") == 0) mods |= GDK_MOD1_MASK;
        else if (strcasecmp(tok, "super") == 0 || strcasecmp(tok, "mod4") == 0) mods |= GDK_MOD4_MASK;

        tok = next;
    }

    free(dup);
    return mods;
}

static const char *get_key_name(const char *keyseq) {
    char *dup = safe_strdup(keyseq);
    char *tok, *saveptr, *last = NULL;

    tok = strtok_r(dup, "+", &saveptr);
    while (tok) {
        last = tok;
        tok = strtok_r(NULL, "+", &saveptr);
    }

    const char *result = last ? safe_strdup(last) : NULL;
    free(dup);
    return result;
}

int keybind_add(KeyBindState *ks, const char *keyseq, const char *commands) {
    if (ks->count >= ks->capacity) {
        int new_cap = ks->capacity == 0 ? 32 : ks->capacity * 2;
        KeyBinding *new_bindings = realloc(ks->bindings, new_cap * sizeof(KeyBinding));
        if (!new_bindings) return -1;
        ks->bindings = new_bindings;
        ks->capacity = new_cap;
    }

    const char *key_name = get_key_name(keyseq);
    if (!key_name) return -1;

    guint keyval = parse_keyval(key_name);
    if (keyval == GDK_KEY_VoidSymbol) {
        fprintf(stderr, "swaynav: unknown key '%s'\n", key_name);
        return -1;
    }

    GdkModifierType mods = parse_mods(keyseq);

    /* Check for existing binding */
    for (int i = 0; i < ks->count; i++) {
        if (ks->bindings[i].keyval == keyval && ks->bindings[i].mods == mods) {
            free(ks->bindings[i].commands);
            ks->bindings[i].commands = safe_strdup(commands);
            return 0;
        }
    }

    ks->bindings[ks->count].keyval = keyval;
    ks->bindings[ks->count].mods = mods;
    ks->bindings[ks->count].commands = safe_strdup(commands);
    ks->count++;

    return 0;
}

void keybind_clear(KeyBindState *ks) {
    for (int i = 0; i < ks->count; i++) {
        free(ks->bindings[i].commands);
    }
    free(ks->bindings);
    ks->bindings = NULL;
    ks->count = 0;
    ks->capacity = 0;
}

KeyBinding *keybind_find(KeyBindState *ks, guint keyval, GdkModifierType mods) {
    mods &= (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD4_MASK | GDK_MOD5_MASK);

    /* Normalize keyval: if it's uppercase letter and shift is pressed,
     * convert to lowercase and keep shift mask */
    if (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z && (mods & GDK_SHIFT_MASK)) {
        keyval = GDK_KEY_a + (keyval - GDK_KEY_A);
    }

    for (int i = 0; i < ks->count; i++) {
        if (ks->bindings[i].keyval == keyval && ks->bindings[i].mods == mods) {
            return &ks->bindings[i];
        }
    }
    return NULL;
}

void keybind_free(KeyBindState *ks) {
    keybind_clear(ks);
}