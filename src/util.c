#include "util.h"
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

char *expand_path(const char *path) {
    if (path == NULL) return NULL;

    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (home == NULL) return safe_strdup(path);

        size_t len = strlen(home) + strlen(path);
        char *expanded = malloc(len);
        if (expanded == NULL) return NULL;

        snprintf(expanded, len, "%s%s", home, path + 1);
        return expanded;
    }

    return safe_strdup(path);
}

int parse_value(int base, const char *str, float default_pct) {
    float val;

    if (str == NULL || str[0] == '\0') {
        val = default_pct;
    } else if (sscanf(str, "%f", &val) <= 0) {
        val = default_pct;
    }

    /* > 1 means pixels, 0-1 means percentage */
    if (val > 1.0f) {
        return (int)val;
    }

    return (int)(base * val);
}

char *trim(char *str) {
    if (str == NULL) return NULL;

    /* Trim leading whitespace */
    while (*str && isspace(*str)) str++;

    if (*str == '\0') return str;

    /* Trim trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';

    return str;
}

char *safe_strdup(const char *str) {
    if (str == NULL) return NULL;
    char *dup = strdup(str);
    if (dup == NULL) {
        fprintf(stderr, "swaynav: strdup failed\n");
        exit(1);
    }
    return dup;
}

int check_ydotoold(void) {
    /* Check if ydotoold socket exists */
    struct stat st;
    if (stat("/tmp/ydotoold.sock", &st) == 0) {
        return 1;
    }

    /* Also check common alternative paths */
    const char *paths[] = {
        "/run/ydotoold.sock",
        "/var/run/ydotoold.sock",
        NULL
    };

    for (int i = 0; paths[i]; i++) {
        if (stat(paths[i], &st) == 0) return 1;
    }

    return 0;
}