#include "mouse.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int run_ydotool_async(const char *tool, const char *arg1, const char *arg2) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        /* Child process */
        if (arg2) execlp("ydotool", "ydotool", tool, arg1, arg2, NULL);
        else if (arg1) execlp("ydotool", "ydotool", tool, arg1, NULL);
        else execlp("ydotool", "ydotool", tool, NULL);
        exit(1);
    }
    /* Parent - don't wait, let child run async */
    return 0;
}

int mouse_move(int x, int y) {
    char sx[16], sy[16];
    snprintf(sx, sizeof(sx), "%d", x);
    snprintf(sy, sizeof(sy), "%d", y);

    return run_ydotool_async("mousemove", sx, sy);
}

int mouse_click(int button) {
    char sb[16];
    snprintf(sb, sizeof(sb), "%d", button);
    return run_ydotool_async("click", sb, NULL);
}

int mouse_doubleclick(int button) {
    char sb[16];
    snprintf(sb, sizeof(sb), "%d", button);
    run_ydotool_async("click", sb, NULL);
    usleep(50000);
    return run_ydotool_async("click", sb, NULL);
}

int mouse_drag_start(int button) {
    char sb[16];
    snprintf(sb, sizeof(sb), "%d", button);
    return run_ydotool_async("mousedown", sb, NULL);
}

int mouse_drag_end(int button) {
    char sb[16];
    snprintf(sb, sizeof(sb), "%d", button);
    return run_ydotool_async("mouseup", sb, NULL);
}