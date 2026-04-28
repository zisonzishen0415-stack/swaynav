#include "history.h"
#include <string.h>

void history_init(History *h) {
    h->cursor = 0;
    h->count = 0;
    memset(h->entries, 0, sizeof(h->entries));
}

void history_save(History *h, GridRect *rect) {
    h->entries[h->cursor] = *rect;
    h->cursor++;
    if (h->cursor >= HISTORY_MAX) {
        h->cursor = 0;
    }
    if (h->count < HISTORY_MAX) {
        h->count++;
    }
}

void history_back(History *h, GridRect *rect) {
    if (h->count == 0) return;

    h->cursor--;
    if (h->cursor < 0) {
        h->cursor = HISTORY_MAX - 1;
    }

    if (h->count > 0) {
        h->count--;
        *rect = h->entries[h->cursor];
    }
}

void history_clear(History *h) {
    h->cursor = 0;
    h->count = 0;
}