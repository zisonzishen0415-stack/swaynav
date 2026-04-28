#!/bin/bash
# swaynav startup script - starts daemon if not running, sends command if running

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SWAYNAV_BIN="$SCRIPT_DIR/bin/swaynav"

# Build if needed
if [ ! -f "$SWAYNAV_BIN" ]; then
    cd "$SCRIPT_DIR"
    make
fi

# Check for ydotoold
if ! pgrep -x "ydotoold" > /dev/null; then
    echo "warning: ydotoold not running, mouse control won't work"
    echo "Start it with: sudo ydotoold"
fi

# Run swaynav (will start daemon or send command to existing daemon)
exec "$SWAYNAV_BIN" "$@"