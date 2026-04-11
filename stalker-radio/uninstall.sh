#!/usr/bin/env bash
set -euo pipefail

echo "Removing stalker-radio..."
sudo rm -f /usr/bin/stalker-radio
sudo rm -f /usr/share/applications/stalker-radio.desktop
sudo rm -f /usr/share/icons/hicolor/256x256/apps/stalker-radio.png
sudo update-desktop-database /usr/share/applications 2>/dev/null || true
echo "Done. Goodbye, Stalker."
