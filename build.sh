flags="-fsanitize=address -g3"
deps="`sdl2-config --cflags --libs` -lm"

cc tmp.c map.c $flags $deps
