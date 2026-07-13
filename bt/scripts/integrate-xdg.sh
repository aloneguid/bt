#!/bin/sh
set -e

# To find default web browser: xdg-settings get default-web-browser

BT_PATH=$1

if [ -z "$BT_PATH" ]; then
    echo "Usage: $0 <executable-path>"
    exit 1
fi

if [ ! -f "$BT_PATH" ]; then
    echo "Error: file '$BT_PATH' not found."
    exit 1
fi

BT_PATH=$(realpath "$BT_PATH")

# Create .desktop file
DESKTOP_FILE="/usr/share/applications/btdev.desktop"
echo "Creating desktop entry at $DESKTOP_FILE"
cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Type=Application
Name=Browser Tamer Dev
Exec=$BT_PATH %u
Icon=BrowserTamerDev
Terminal=false
Categories=Network;WebBrowser;
MimeType=text/html;text/xml;application/xhtml+xml;x-scheme-handler/http;x-scheme-handler/https;
Comment=Choose the right browser for each URL.
EOF

echo "Installing alternatives for $BT_PATH"
update-alternatives --install /usr/bin/x-www-browser x-www-browser "$BT_PATH" 100
update-alternatives --install /usr/bin/gnome-www-browser gnome-www-browser "$BT_PATH" 100

if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database -q
fi
