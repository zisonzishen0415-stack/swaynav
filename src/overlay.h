#ifndef SWAYNAV_OVERLAY_H
#define SWAYNAV_OVERLAY_H

#include <gtk/gtk.h>
#include "commands.h"

/* Create the overlay window */
GtkWidget *overlay_create(AppState *as);

/* Show overlay */
void overlay_show(GtkWidget *overlay);

/* Hide overlay */
void overlay_hide(GtkWidget *overlay);

/* Redraw the grid */
void overlay_redraw(GtkWidget *overlay, AppState *as);

/* Get current monitor/output index */
int overlay_get_current_output(GtkWidget *overlay);

#endif /* SWAYNAV_OVERLAY_H */