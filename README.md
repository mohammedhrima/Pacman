# Pacman

A small Pacman written in C with SDL2. Move through the map, collect every `C`, avoid the enemies, and
reach the exit `E` once everything is collected. A mini-map in the top-right shows the whole layout and
the enemies moving in real time.

The map is loaded from a `.ber` file — a grid of `1` walls, `0` paths, `P` player, `C` collectibles,
`X` enemies and `E` exit — and is checked for a solvable path before the game starts.

## Build and run

```sh
./build.sh
./pacman      # reads ./001.ber
```

Needs SDL2 (`brew install sdl2` on macOS). The window sizes itself to your screen and keeps the player
centered.

## Controls

- Arrow keys — move
- Esc — quit
