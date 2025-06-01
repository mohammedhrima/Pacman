#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define SCALE 60
// EVENTS
#define RELEASE SDL_KEYUP
#define PRESS SDL_KEYDOWN
#define QUIT SDL_QUIT
#define MOUSE_MOTION SDL_MOUSEMOTION
#define MOUSE_DOWN SDL_MOUSEBUTTONDOWN
#define MOUSE_UP SDL_MOUSEBUTTONUP
#define MOUSE_SCROLL SDL_MOUSEWHEEL
// KEYS
#define ESC 27
#define SPACE 32
#define RESET 114
#define UP 1073741906
#define DOWN 1073741905
#define LEFT 1073741904
#define RIGHT 1073741903
#define FORWARD 1073741920
#define BACKWARD 1073741914
#define MOUSE_LEFT SDL_BUTTON_LEFT

// COLORS
#define SDL_COLOR(r, g, b) r << 16 | g << 8 | b
#define RED SDL_COLOR(255, 0, 0)
#define GREEN SDL_COLOR(0, 255, 0)
#define BLUE SDL_COLOR(0, 0, 255)
#define ORANGE SDL_COLOR(237, 106, 18)
#define YELLOW SDL_COLOR(255, 255, 0)
#define WHITE SDL_COLOR(255, 255, 255)
#define BACKGROUND SDL_COLOR(147, 255, 225)
#define COLOR1 SDL_COLOR(136, 235, 0)
#define COLOR2 SDL_COLOR(235, 206, 0)
#define COLOR3 SDL_COLOR(235, 1, 9)
#define COLOR4 SDL_COLOR(205, 0, 235)

#define PI 3.14159
#define DEG(rad) (rad / 180) * PI

typedef struct
{
    int x;
    int y;

    int next_x;
    int next_y;

    float real_x;
    float real_y;
} Coor;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Uint32 *pixels;
    SDL_Event event;
    char *map;
    char *visited;

    Coor player;
    Coor exit_;
    Coor *collectibles;
    int collectibles_pos;
    int collectibles_len;

    Coor *enemies;
    int enemies_len;
    int enemies_pos;

    bool quit;
    bool map_parsed;
    int key;
    float angle;

    int map_width;
    int map_height;

    int window_width;
    int window_height;
} Win;

// FUNCTIONS
void init(Win *win);
void close_window(Win *win);
char *readline(Win *win, int fd);
void parse_map(Win *win, int fd);
void update_window(Win *win);
void check_map(Win *win);
void print_map(char *msg, char *map, int width, int height);

#endif