flags="-fsanitize=address -g3"
deps="`sdl2-config --cflags --libs` -I`sdl2-config --prefix`/include -lm"

cc Pacman.c $flags $deps -o pacman
