#ifndef SWAYNAV_SWAYIPC_H
#define SWAYNAV_SWAYIPC_H

#include <json-c/json.h>

/* Output information from Sway */
typedef struct {
    int x;
    int y;
    int w;
    int h;
    char *name;
    int active;
} SwayOutput;

/* Window information from Sway */
typedef struct {
    int x;
    int y;
    int w;
    int h;
    char *name;
    int focused;
} SwayWindow;

/* IPC message types */
#define IPC_GET_OUTPUTS    3
#define IPC_GET_TREE       4
#define IPC_GET_INPUTS     9
#define IPC_GET_SEATS      12

/* Check if running under Sway */
int swayipc_is_sway(void);

/* Get ydotoold device pointer_accel value, returns -999 if not found/error */
float swayipc_get_ydotoold_accel(void);

/* Set ydotoold device pointer_accel to 0, returns 0 on success */
int swayipc_set_ydotoold_accel_zero(void);

/* Connect to Sway IPC socket */
int swayipc_connect(void);

/* Close connection */
void swayipc_close(int sock);

/* Send command and get response */
json_object *swayipc_send(int sock, int type, const char *payload);

/* Get outputs (monitors) */
SwayOutput *swayipc_get_outputs(int *count);

/* Get focused window */
SwayWindow *swayipc_get_focused_window(void);

/* Get cursor position */
int swayipc_get_cursor_position(int *x, int *y);

/* Free output array */
void swayipc_free_outputs(SwayOutput *outputs, int count);

/* Free window */
void swayipc_free_window(SwayWindow *win);

#endif /* SWAYNAV_SWAYIPC_H */