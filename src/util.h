#ifndef SWAYNAV_UTIL_H
#define SWAYNAV_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Expand ~ in path to home directory */
char *expand_path(const char *path);

/* Parse percentage or pixel value */
int parse_value(int base, const char *str, float default_pct);

/* String trimming */
char *trim(char *str);

/* Safe string duplication */
char *safe_strdup(const char *str);

/* Check if ydotoold is running */
int check_ydotoold(void);

#endif /* SWAYNAV_UTIL_H */