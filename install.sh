#!/bin/bash
# swaynav installation script - one-click setup

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== swaynav installer ==="

# Stop running swaynav first
if pgrep -x swaynav > /dev/null; then
    echo "Stopping running swaynav..."
    pkill -x swaynav
    sleep 1
fi

# Build
echo "Building swaynav..."
cd "$SCRIPT_DIR"
make clean
make
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi
echo "Build successful."

# Install binary
echo "Installing binary to ~/.local/bin..."
mkdir -p ~/.local/bin
cp bin/swaynav ~/.local/bin/swaynav

# Install config
mkdir -p ~/.config/swaynav
if [ ! -f ~/.config/swaynav/config ]; then
    echo "Creating default config..."
    cp data/config.example ~/.config/swaynav/config
fi

# Setup ydotoold system service (requires sudo)
echo ""
echo "=== ydotoold daemon setup ==="
echo "ydotoold is required for mouse control."
echo ""

# Always update service file to ensure latest fixes
echo "Updating ydotoold service (requires sudo)..."
sudo cp data/ydotoold.service /etc/systemd/system/
sudo systemctl daemon-reload

if systemctl is-active --quiet ydotoold; then
    echo "Restarting ydotoold to apply updates..."
    sudo systemctl restart ydotoold
else
    echo "Starting ydotoold service..."
    sudo systemctl enable ydotoold
    sudo systemctl start ydotoold
fi

# Wait for socket and verify permissions
sleep 1
SOCKET_PERMS=$(stat -c "%a" /tmp/.ydotool_socket 2>/dev/null || echo "missing")
if [ "$SOCKET_PERMS" = "missing" ]; then
    echo "WARNING: Socket not found, ydotoold may not be running correctly"
elif [ "$SOCKET_PERMS" != "666" ]; then
    echo "Socket permissions: $SOCKET_PERMS, fixing..."
    sudo chmod 666 /tmp/.ydotool_socket
fi

echo "ydotoold ready."

# Setup swaynav user service
echo ""
echo "=== swaynav autostart setup ==="

SYSTEMD_USER_DIR=~/.config/systemd/user
mkdir -p "$SYSTEMD_USER_DIR"

# Update service file with correct path
cat > "$SYSTEMD_USER_DIR/swaynav.service" << 'EOF'
[Unit]
Description=swaynav - keyboard-driven mouse control daemon
Documentation=https://github.com/zisonzishen0415-stack/swaynav
After=graphical.target

[Service]
Type=simple
ExecStart=%h/.local/bin/swaynav
Restart=on-failure

[Install]
WantedBy=default.target
EOF

systemctl --user daemon-reload
systemctl --user enable swaynav

echo "swaynav user service enabled."

# Check if sway config needs update
SWAY_CONFIG=~/.config/sway/config
if [ -f "$SWAY_CONFIG" ]; then
    if ! grep -q "swaynav" "$SWAY_CONFIG"; then
        echo ""
        echo "=== Sway config ==="
        echo "Adding swaynav to Sway config..."
        echo "" >> "$SWAY_CONFIG"
        echo "# swaynav - keyboard mouse control" >> "$SWAY_CONFIG"
        echo "exec ~/.local/bin/swaynav" >> "$SWAY_CONFIG"
        echo "bindsym ctrl+semicolon exec ~/.local/bin/swaynav toggle" >> "$SWAY_CONFIG"
        echo "Added to $SWAY_CONFIG"
        echo ""
        echo "Reload Sway config: swaymsg reload"
    else
        echo "Sway config already has swaynav."
        # Update paths if using old project path
        if grep -q "/home/zison/development/swaynav" "$SWAY_CONFIG"; then
            sed -i 's|/home/zison/development/swaynav/bin/swaynav|~/.local/bin/swaynav|g' "$SWAY_CONFIG"
            echo "Updated paths in Sway config."
            echo "Reload Sway config: swaymsg reload"
        fi
    fi
fi

echo ""
echo "=== Installation complete ==="
echo ""
echo "swaynav will start automatically on next login."
echo ""
echo "Start now: systemctl --user start swaynav"
echo "Or reload Sway: swaymsg reload"
echo ""
echo "Test: Press Ctrl+; to activate swaynav"