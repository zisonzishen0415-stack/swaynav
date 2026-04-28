#ifndef SWAYNAV_HISTORY_H
#define SWAYNAV_HISTORY_H

#include "grid.h"

#define HISTORY_MAX 100

typedef struct {
    GridRect entries[HISTORY_MAX];
    int cursor;
    int count;
} History;

void history_init(History *h);
void history_save(History *h, GridRect *rect);
void history_back(History *h, GridRect *rect);
void history_clear(History *h);

#endif /* SWAYNAV_HISTORY_H */