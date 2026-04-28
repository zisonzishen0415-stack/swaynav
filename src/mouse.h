#ifndef SWAYNAV_MOUSE_H
#define SWAYNAV_MOUSE_H

/* Mouse button constants */
#define MOUSE_LEFT     1
#define MOUSE_MIDDLE   2
#define MOUSE_RIGHT    3
#define MOUSE_SCROLL_U 4
#define MOUSE_SCROLL_D 5
#define MOUSE_SCROLL_L 6
#define MOUSE_SCROLL_R 7

/* Move mouse to position */
int mouse_move(int x, int y);

/* Click mouse button */
int mouse_click(int button);

/* Double click */
int mouse_doubleclick(int button);

/* Start/end drag */
int mouse_drag_start(int button);
int mouse_drag_end(int button);

/* Get current mouse position */
int mouse_get_position(int *x, int *y);

#endif /* SWAYNAV_MOUSE_H */