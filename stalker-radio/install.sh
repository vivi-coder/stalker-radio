#!/usr/bin/env bash
set -euo pipefail

# ─────────────────────────────────────────────
#  STALKER: Anomaly — PDA Radio Scanner
#  Arch Linux installer
# ─────────────────────────────────────────────

BOLD="\033[1m"
GREEN="\033[1;32m"
CYAN="\033[1;36m"
YELLOW="\033[1;33m"
RED="\033[1;31m"
RESET="\033[0m"

log()  { echo -e "${CYAN}[stalker-radio]${RESET} $*"; }
ok()   { echo -e "${GREEN}[  OK  ]${RESET} $*"; }
warn() { echo -e "${YELLOW}[ WARN ]${RESET} $*"; }
die()  { echo -e "${RED}[ FAIL ]${RESET} $*"; exit 1; }

echo -e "${BOLD}"
echo "  ◆ STALKER: Anomaly — PDA Radio Scanner"
echo "  Arch Linux / Hyprland installer"
echo -e "${RESET}"

# ── Dependency check ──
log "Checking dependencies..."

MISSING=()
command -v cmake   &>/dev/null || MISSING+=(cmake)
command -v make    &>/dev/null || MISSING+=(make)
command -v pkg-config &>/dev/null || MISSING+=(pkgconf)
pkg-config --exists gtk4 2>/dev/null || MISSING+=(gtk4)

if [ ${#MISSING[@]} -gt 0 ]; then
    warn "Missing packages: ${MISSING[*]}"
    log "Installing with pacman..."
    sudo pacman -S --needed --noconfirm "${MISSING[@]}" || die "pacman failed"
fi

ok "All dependencies satisfied."

# ── Build ──
BUILD_DIR="build"
log "Configuring with CMake..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

log "Building..."
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
ok "Build complete."

# ── Install ──
log "Installing to /usr/bin and /usr/share..."
sudo cmake --install "$BUILD_DIR"
ok "Installed stalker-radio to /usr/bin/stalker-radio"

# ── Desktop integration ──
log "Updating desktop database..."
sudo update-desktop-database /usr/share/applications 2>/dev/null || true

echo ""
echo -e "${GREEN}${BOLD}Installation complete.${RESET}"
echo ""
echo -e "  Run from terminal:  ${CYAN}stalker-radio${RESET}"
echo -e "  Or launch from your app menu / wofi / rofi."
echo ""
