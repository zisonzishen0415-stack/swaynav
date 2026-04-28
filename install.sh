#!/bin/bash
# swaynav installation script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== swaynav installer ==="

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

# Install config example
mkdir -p ~/.config/swaynav
cp data/config.example ~/.config/swaynav/config.example

# Create default config if not exists
if [ ! -f ~/.config/swaynav/config ]; then
    echo "Creating default config..."
    cp data/config.example ~/.config/swaynav/config
fi

echo ""
echo "=== Installation complete ==="
echo ""
echo "Next steps:"
echo ""
echo "1. Ensure ~/.local/bin is in your PATH (add to ~/.bashrc or ~/.profile):"
echo "   export PATH=\"\$PATH:~/.local/bin\""
echo ""
echo "2. Add to your Sway config (~/.config/sway/config):"
echo "   # swaynav - keyboard mouse control"
echo "   bindsym ctrl+semicolon exec swaynav start"
echo ""
echo "3. Start ydotoold daemon (required for mouse control):"
echo "   sudo ydotoold"
echo ""
echo "4. Reload Sway config:"
echo "   swaymsg reload"
echo ""
echo "Usage:"
echo "   Press Ctrl+; to activate swaynav"
echo "   h/j/k/l = cut selection (shrink)"
echo "   shift+h/j/k/l = move selection"
echo "   space/Return = warp cursor and click"
echo "   w = warp cursor only"
echo "   Escape = end"
echo ""