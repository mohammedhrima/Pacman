#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

// COLORS (cohesive dark theme, low glare; all entities visibly distinct)
#define BACKGROUND 0x16161f // floor / frame clear (soft dark navy)
#define PANEL 0x0e0e16		// mini-map panel
#define BLUE 0x2b4cf0		// walls
#define YELLOW 0xffe14d		// pacman
#define PELLET 0xf5e9dc		// collectibles (soft white-peach, not yellow)
#define GREEN 0x3ddc84		// exit open
#define LOCKED 0x8a8f98		// exit locked (muted slate, not red)
#define WHITE 0xeaeaea		// eyes / mini-map border
// enemy palette (stable per-enemy; none near pacman yellow)
#define GHOST_RED 0xe5484d
#define GHOST_PINK 0xff8fc7
#define GHOST_CYAN 0x49d0e0
#define GHOST_PURPLE 0xb15cff

// KEYS
#define RIGHT SDLK_RIGHT
#define DOWN SDLK_DOWN
#define LEFT SDLK_LEFT
#define UP SDLK_UP

// STRUCTURES
typedef struct Game Game;
typedef struct App App;
typedef struct Pair Pair;

struct Game
{
	int Player;
	int Exit;
	int *Collectible;
	int *Enemies;
	int Collectlen;
	int Enemlen;
	char *map;
};

struct App
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint32 *pixels;
	SDL_Event event;
	int quit;
	char *addr;
	int bits_per_pixel;
	int line_length;
	Game game;
};

struct Pair
{
	int p;
	int prev;
};

// map parsing
void error_exit(char *msg);
char *strjoin(char *s1, char *s2);
char *read_line(int fd);
int *join_2D(int *arr, int to_add, int len);
int map_contains_allowed_chars(char *str, Game *game, int w, int h);
int is_wall(char *str);
Game get_map(int fd);
int is_access(char c);
int move(int Player, Game game, int w, int h, char **was_here);
char *get_to_the_target(Game game, int fd);

// drawing
void put_one_pixel(App *var, int x, int y, int color);
void put_pixels(App *var, int x_start, int y_start, int width, int height, int color);
int clicked_key(int keycode, App *var);
int pow2(int num);
int face(App *var, int x_center, int y_center, int radius, int color, float frequence);
int circle(App *var, int x_center, int y_center, int radius, int color);
int head(App *var, int x_center, int y_center, int radius, int color);
void draw_enemy(App *var, int x, int y, int radius, int color);

// enemies + frame
void go(int c, int dx, int dy, Pair *q, int *r, int *visited);
void move_enemie(App *var, int x, int y, int last);
void draw_pacman(App *var, int x, int y, int radius);
void draw_map(App *var);
void draw_mini_map(App *var);
int draw(App *var);

#endif
