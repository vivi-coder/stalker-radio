# STALKER: Anomaly — PDA Radio Scanner

A native GTK4 desktop app for Arch Linux / Hyprland that simulates the Zone's radio traffic — faction squad chatter, VIPER helicopter air wing comms, anomaly warnings, and interference noise. Looks and feels like the in-game PDA.

![preview](assets/preview.png)

---

## Features

- **6 ground factions** — Loners, Duty, Freedom, Mercs, Bandits, Ecologists, each with named stalkers and their own frequency
- **VIPER air wing** — VIPER-1 through VIPER-5 with roles (Attack, Scout, Gunship, Medevac), live altitude/speed readout, and dedicated air comms including combat calls
- **Message types** — routine chatter, warnings, critical danger alerts, system messages, air transmissions
- **Filters** — All / Air / Danger / Chatter
- **Live signal bars** that change color for air vs ground transmissions
- **Noise bursts** — random static lines between messages
- **CRT terminal aesthetic** — monospace, green-on-black, no gradients, no nonsense
- Click any squad or VIPER to tune the frequency display to their channel

---

## Requirements

- Arch Linux (or any distro with GTK4)
- Hyprland, Sway, GNOME, KDE — anything with Wayland or X11 works
- `gtk4`, `cmake`, `make`, `pkgconf`

---

## Install (Arch Linux)

```bash
git clone https://github.com/yourname/stalker-radio
cd stalker-radio
chmod +x install.sh
./install.sh
```

The script will:
1. Check for and install missing pacman dependencies
2. Build with CMake in Release mode
3. Install the binary to `/usr/bin/stalker-radio`
4. Register a `.desktop` file so it shows up in wofi/rofi/app menus

---

## Manual build (no install)

```bash
# Install deps
sudo pacman -S --needed gtk4 cmake pkgconf

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run directly
./build/stalker-radio
```

---

## Uninstall

```bash
./uninstall.sh
```

---

## Hyprland window rules (optional)

Add to `~/.config/hypr/hyprland.conf` if you want it to float at a fixed size:

```
windowrulev2 = float, class:^(stalker-radio)$
windowrulev2 = size 1100 700, class:^(stalker-radio)$
windowrulev2 = center, class:^(stalker-radio)$
```

---

## Project structure

```
stalker-radio/
├── src/
│   └── main.cpp          # Full GTK4 app — UI, data, timers
├── assets/
│   ├── stalker-radio.desktop
│   └── stalker-radio.png  # (optional icon)
├── CMakeLists.txt
├── install.sh
├── uninstall.sh
└── README.md
```

---

## License

MIT. Do what you want, Stalker.
