#include "header.h"

static int KEY = RIGHT;
static double SCALE = 120;
static int WINDOW_WIDTH;
static int WINDOW_HEIGHT;
static int SCREEN_WIDTH = 1920; // set to the display's usable size at startup
static int SCREEN_HEIGHT = 1080;
static int pacman_dx = 0;
static int pacman_dy = 0;
static int paused = 0; // space toggles: freeze movement, keep animating

void error_exit(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

char *strjoin(char *s1, char *s2)
{
	if (!s1 && !s2)
		return (NULL);
	if (!s1)
		return (strdup(s2));
	if (!s2)
		return (strdup(s1));
	char *str = calloc(strlen(s1) + strlen(s2) + 1, 1);
	if (!str)
		return (NULL);
	strcpy(str, s1);
	strcat(str, s2);
	return (str);
}

char *read_line(int fd)
{
	char *str;
	char *tmp;
	char buffer[2];

	memset(buffer, 0, 2);
	str = NULL;
	int r = read(fd, buffer, 1);
	while (r > 0 && buffer[0] != '\n')
	{
		buffer[r] = '\0';
		tmp = strjoin(str, buffer);
		memset(buffer, 0, 2);
		free(str);
		str = tmp;
		r = read(fd, buffer, 1);
	}
	if (r > 0 && !str)
		str = strdup(buffer);
	return (str);
}

int *join_2D(int *arr, int to_add, int len)
{
	int *res;
	int i;

	res = malloc((len + 1) * sizeof(int));
	if (!res)
		exit(-1);
	i = 0;
	while (i < len - 1)
	{
		res[i] = arr[i];
		i++;
	}
	res[i] = to_add;
	free(arr);
	return (res);
}

int map_contains_allowed_chars(char *str, Game *game, int w, int h)
{
	int i = 0;
	if (str[i] != '1')
		return (-1);
	while (str[i] && str[i] != '\n')
	{
		if (!strchr("01PCEX", str[i]))
			return (-2);
		if (str[i] == 'P')
		{
			if (game->Player)
				return (-3);
			game->Player = h * w + i;
		}
		if (str[i] == 'C')
		{
			game->Collectlen++;
			game->Collectible = join_2D(game->Collectible, h * w + i, game->Collectlen);
		}
		if (str[i] == 'X')
		{
			game->Enemlen++;
			game->Enemies = join_2D(game->Enemies, h * w + i, game->Enemlen);
		}
		if (str[i] == 'E')
		{
			if (game->Exit)
				return (-4);
			game->Exit = h * w + i;
		}
		i++;
	}
	if (str[i - 1] != '1')
		return (-1);
	return (0);
}

int is_wall(char *str)
{
	int i;

	i = 0;
	while (str && str[i] && str[i] == '1')
		i++;
	if (i > 0 && (str[i] == '\n' || str[i] == '\0'))
		return (0);
	else
		return (-1);
}

Game get_map(int fd)
{
	Game game;
	game.Player = 0;
	game.Exit = 0;
	game.Collectible = NULL;
	game.Collectlen = 0;
	game.Enemies = NULL;
	game.Enemlen = 0;
	game.map = NULL;
	char *tmp;

	char *str = read_line(fd);
	WINDOW_WIDTH = 0;
	WINDOW_HEIGHT = 0;
	char *arr[] = {
		 "",
		 "map is invalid, verify walls",
		 "map is invalid, it contains unknown characters",
		 "map is invalid, it contains more than one Player",
		 "map is invalid, it contains more than one Exit"};
	if (!str)
		error_exit("something wrong with the map");
	if (is_wall(str) < 0)
		error_exit("map is invalid, verify first line if is wall");
	WINDOW_WIDTH = strlen(str);
	while (str && strlen(str))
	{
		if (strlen(str) != (size_t)WINDOW_WIDTH)
			error_exit("map is invalid, all lines doesn't have the same width");
		int n = map_contains_allowed_chars(str, &game, WINDOW_WIDTH, WINDOW_HEIGHT);
		if (n == -1 || n == -2 || n == -3 || n == -4)
			error_exit(arr[-n]);
		tmp = strjoin(game.map, str);
		free(game.map);
		free(str);
		game.map = tmp;
		str = read_line(fd);
		if (str && str[0] == '\n')
			error_exit("map is invalid, there is two followed new lines");
		WINDOW_HEIGHT++;
	}
	if (is_wall(game.map + WINDOW_HEIGHT * WINDOW_WIDTH - WINDOW_WIDTH) < 0)
		error_exit("map is invalid, verify last line if is wall");
	if (game.Collectlen == 0)
		error_exit("map is invalid, there isn't enough Collectible");
	if (game.Player == 0)
		error_exit("map is invalid, there isn't any Player");
	if (game.Exit == 0)
		error_exit("map is invalid, there isn't any Exit");

	for (int i = 0; game.map[i]; i++)
		if (game.map[i] == 'X')
			game.map[i] = '0';
	return (game);
}

int is_access(char c)
{
	return c && strchr("0CEX", c);
}

int m = 0;
int move(int Player, Game game, int w, int h, char **was_here)
{
	int x = Player % w;
	int y = Player / w;
	int n = 0;

	if ((*was_here)[Player] == 'C')
		m++;
	if ((*was_here)[Player] == 'E')
		(*was_here)[Player] = 'F';
	else
		(*was_here)[Player] = 'P';
	if ((*was_here)[game.Exit] != 'F')
		n = -1;
	if (m != game.Collectlen)
		n = -1;
	if ((*was_here)[Player] == 'P' && is_access((*was_here)[y * w + x + 1]) && n < 0)
		n = move(y * w + x + 1, game, w, h, was_here);
	if ((*was_here)[Player] == 'P' && is_access((*was_here)[y * w + x - 1]) && n < 0)
		n = move(y * w + x - 1, game, w, h, was_here);
	if ((*was_here)[Player] == 'P' && is_access((*was_here)[(y + 1) * w + x]) && n < 0)
		n = move((y + 1) * w + x, game, w, h, was_here);
	if ((*was_here)[Player] == 'P' && is_access((*was_here)[(y - 1) * w + x]) && n < 0)
		n = move((y - 1) * w + x, game, w, h, was_here);
	return (n);
}

char *get_to_the_target(Game game, int fd)
{
	char *map_copy;

	(void)fd;
	map_copy = strdup(game.map);
	int n = move(game.Player, game, WINDOW_WIDTH, WINDOW_HEIGHT, &map_copy);
	if (n < 0)
	{
		printf("Path is not valid\n");
		free(map_copy);
		return (NULL);
	}
	printf("Path is valid\n");
	return (map_copy);
}

void put_one_pixel(App *var, int x, int y, int color)
{
	char *dst;

	dst = var->addr + (y * var->line_length + x * (var->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

void put_pixels(App *var, int x_start, int y_start, int width, int height, int color)
{
	int x_end = x_start + width;
	int y_end = y_start + height;

	if (x_start < 0)
		x_start = 0;
	if (x_end > SCREEN_WIDTH)
		x_end = SCREEN_WIDTH;
	if (y_start < 0)
		y_start = 0;
	if (y_end > SCREEN_HEIGHT)
		y_end = SCREEN_HEIGHT;
	for (int y = y_start; y < y_end; y++)
		for (int x = x_start; x < x_end; x++)
			put_one_pixel(var, x, y, color);
}

int clicked_key(int keycode, App *var)
{
	(void)var;
	KEY = keycode;
	if (keycode == LEFT)
		pacman_dx = -1, pacman_dy = 0;
	else if (keycode == RIGHT)
		pacman_dx = 1, pacman_dy = 0;
	else if (keycode == UP)
		pacman_dy = -1, pacman_dx = 0;
	else if (keycode == DOWN)
		pacman_dy = 1, pacman_dx = 0;
	return (0);
}

int pow2(int num)
{
	return (num * num);
}

int face(App *var, int x_center, int y_center, int radius, int color, float frequence)
{
	static int last_key;

	if (KEY != RIGHT && KEY != LEFT && KEY != UP && KEY != DOWN)
	{
		if (last_key == RIGHT || last_key == LEFT || last_key == UP || last_key == DOWN)
			KEY = last_key;
		else
			KEY = RIGHT;
	}
	int dx = (KEY == RIGHT) - (KEY == LEFT); // facing x
	int dy = (KEY == DOWN) - (KEY == UP);	 // facing y
	for (int x = x_center - radius; x < x_center + radius; x++)
		for (int y = y_center - radius; y < y_center + radius; y++)
		{
			int fwd = (x - x_center) * dx + (y - y_center) * dy; // forward component
			int side = dx ? (y - y_center) : (x - x_center);	 // sideways component
			if (pow2(x - x_center) + pow2(y - y_center) < pow2(radius) && (fwd < 0 || abs(side) >= abs(fwd) * frequence))
				put_pixels(var, x, y, 1, 1, color);
		}
	last_key = KEY;
	return (0);
}

int circle(App *var, int x_center, int y_center, int radius, int color)
{
	int x;
	int y;

	x = x_center - radius;
	while (x < x_center + radius)
	{
		y = y_center - radius;
		while (y < y_center + radius)
		{
			if (pow2(x_center - x) + pow2(y_center - y) <= pow2(radius))
				put_pixels(var, x, y, 1, 1, color);
			y++;
		}
		x++;
	}
	return (0);
}

int head(App *var, int x_center, int y_center, int radius, int color)
{
	int x;
	int y;

	x = x_center - radius;
	while (x < x_center + radius)
	{
		y = y_center - radius;
		while (y < y_center)
		{
			if (pow2(x_center - x) + pow2(y_center - y) <= pow2(radius) && x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
				put_one_pixel(var, x, y, color);
			y++;
		}
		x++;
	}
	return (0);
}

static float z = 0;		 
static float eye_step = 0.75; 
void draw_enemy(App *var, int x, int y, int radius, int color)
{
	head(var, x + radius, y + radius, radius, color);
	put_pixels(var, x, y + radius, radius * 2, radius, color);
	// eyes
	circle(var, x + radius * 0.6, y + radius / 2, radius / 4, WHITE); // left eye
	circle(var, x + radius * 1.4, y + radius / 2, radius / 4, WHITE); // right eye
	circle(var, x + radius * 0.6 + z, y + radius / 2 + 2, radius / 8, 0);
	circle(var, x + radius * 1.4 + z, y + radius / 2 + 2, radius / 8, 0);
}

static float pacman_vx;
static float pacman_vy;

void go(int c, int dx, int dy, Pair *q, int *r, int *visited)
{
	int x = c % WINDOW_WIDTH + dx;
	int y = c / WINDOW_WIDTH + dy;
	int p = y * WINDOW_WIDTH + x;
	if (!visited[p])
	{
		visited[p] = 1;
		q[*r].p = p;
		int prev = LEFT;
		if (dx < 0)
			prev = RIGHT;
		if (dy < 0)
			prev = DOWN;
		if (dy > 0)
			prev = UP;
		q[*r].prev = prev;
		*r = *r + 1;
	}
}

void move_enemie(App *var, int x, int y, int last)
{
	int *visited = calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(int));
	Pair *q = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(Pair));
	int l = 0;
	int r = 0;

	(void)last;
	q[r].p = y * WINDOW_WIDTH + x;
	q[r].prev = 0;
	r++;
	while (l < r)
	{
		int curr = q[l].p;
		int prev = q[l].prev;

		l++;
		int cx = curr % WINDOW_WIDTH;
		int cy = curr / WINDOW_WIDTH;
		if (cx < 0 || cx >= WINDOW_WIDTH || cy < 0 || cy >= WINDOW_HEIGHT || var->game.map[curr] == '1' || var->game.map[curr] == 'E')
			continue;
		for (int i = 0; i < var->game.Enemlen; i++)
		{
			if (var->game.Enemies[i] == curr)
			{
				int dx = 0, dy = 0;
				if (prev == UP)
					dy--;
				else if (prev == DOWN)
					dy++;
				else if (prev == LEFT)
					dx--;
				else if (prev == RIGHT)
					dx++;
				int ex = var->game.Enemies[i] % WINDOW_WIDTH;
				int ey = var->game.Enemies[i] / WINDOW_WIDTH;

				ex += dx;
				ey += dy;
				int dest = ey * WINDOW_WIDTH + ex;
				int occupied = 0;
				for (int j = 0; j < var->game.Enemlen; j++)
					if (j != i && var->game.Enemies[j] == dest)
						occupied = 1;
				if (!occupied)
				{
					if (dest == var->game.Player)
					{
						printf("Enemie did hit you\n");
						exit(0);
					}
					var->game.Enemies[i] = dest; // enemies live only in Enemies[], not the map grid
				}
			}
		}
		go(curr, 1, 0, q, &r, visited);
		go(curr, -1, 0, q, &r, visited);
		go(curr, 0, 1, q, &r, visited);
		go(curr, 0, -1, q, &r, visited);
	}
	free(q);
	free(visited);
}

static float frequence = 0.5;
static float fr = 0.1;

void draw_pacman(App *var, int x, int y, int radius)
{
	frequence += fr;
	if (frequence > 1 || frequence < 0)
		fr = -fr;
	face(var, x + radius, y + radius, radius, YELLOW, frequence);
}

static float R = 1;
float r = 0.01;
void draw_map(App *var)
{
	int i = 0;
	int j = 0;
	int pac_i = 0;
	int pac_j = 0;

	R += r;
	if (R >= 1.1 || R <= 0.9)
		r = -r;
	while (i < WINDOW_HEIGHT)
	{
		j = 0;
		while (j < WINDOW_WIDTH)
		{
			if (var->game.map[i * WINDOW_WIDTH + j] == 'P')
			{
				pac_i = i;
				pac_j = j;
			}
			j++;
		}
		i++;
	}
	static int first_frame = 1;
	if (first_frame)
	{
		pacman_vx = pac_j * SCALE;
		pacman_vy = pac_i * SCALE;
		first_frame = 0;
	}
	float speed = 1.0 / 60 * 10;
	pacman_vx += (pac_j * SCALE - pacman_vx) * speed;
	pacman_vy += (pac_i * SCALE - pacman_vy) * speed;

	int pac_x = SCREEN_WIDTH / 2;
	int pac_y = SCREEN_HEIGHT / 2;
	for (int ci = pac_i - 100; ci < pac_i + 100; ci++)
	{
		for (int cj = pac_j - 100; cj < pac_j + 100; cj++)
		{
			int s = SCALE;
			float x = pac_x + (cj * SCALE - pacman_vx);
			float y = pac_y + (ci * SCALE - pacman_vy);

			if (ci < 0 || cj < 0 || ci >= WINDOW_HEIGHT || cj >= WINDOW_WIDTH)
				put_pixels(var, x, y, s, s, BACKGROUND);
			else if (var->game.map[ci * WINDOW_WIDTH + cj] == '1')
				put_pixels(var, x, y, s, s, BLUE);
			else if (var->game.map[ci * WINDOW_WIDTH + cj] == 'E')
			{
				if (var->game.Collectlen)
					put_pixels(var, x, y, s, s, LOCKED);
				else
					put_pixels(var, x + s * 0.1, y + s * 0.1, s * 0.8, s * 0.8, GREEN);
			}
			else if (var->game.map[ci * WINDOW_WIDTH + cj] == 'C')
				circle(var, x + SCALE / 2, y + SCALE / 2, R * SCALE / 4, PELLET);
		}
	}
	draw_pacman(var, pac_x, pac_y, SCALE / 2);

	// enemies
	static float e_x[50];
	static float e_y[50];
	z += eye_step; // advance the shared eye pupil once per frame (count-independent, slow)
	if (z >= 4 || z <= -4)
		eye_step = -eye_step;
	int ghosts[4] = {GHOST_RED, GHOST_PINK, GHOST_CYAN, GHOST_PURPLE};
	for (int k = 0; k < var->game.Enemlen; k++)
	{
		int ex = var->game.Enemies[k] % WINDOW_WIDTH;
		int ey = var->game.Enemies[k] / WINDOW_WIDTH;
		float s = speed; // glide tuned to the slower (f%32) enemy cadence -> smooth
		e_x[k] += (ex * SCALE - e_x[k]) * s;
		e_y[k] += (ey * SCALE - e_y[k]) * s;
		draw_enemy(var, pac_x + (e_x[k] - pacman_vx), pac_y + (e_y[k] - pacman_vy), SCALE / 2, ghosts[k % 4]);
	}
}

void draw_mini_map(App *var)
{
	int margin = 15;
	int ms = (SCREEN_WIDTH / 3) / WINDOW_WIDTH; // px per tile, auto-fit
	if ((SCREEN_HEIGHT / 3) / WINDOW_HEIGHT < ms)
		ms = (SCREEN_HEIGHT / 3) / WINDOW_HEIGHT;
	if (ms < 1)
		ms = 1;
	int mm_w = WINDOW_WIDTH * ms;
	int mm_h = WINDOW_HEIGHT * ms;
	int x0 = SCREEN_WIDTH - mm_w - margin; // top-right
	int y0 = margin;

	put_pixels(var, x0 - 2, y0 - 2, mm_w + 4, mm_h + 4, WHITE); // border
	put_pixels(var, x0, y0, mm_w, mm_h, PANEL);				   // panel bg
	for (int h = 0; h < WINDOW_HEIGHT; h++)
		for (int w = 0; w < WINDOW_WIDTH; w++)
		{
			char c = var->game.map[h * WINDOW_WIDTH + w];
			int cx = x0 + w * ms;
			int cy = y0 + h * ms;
			if (c == '1')
				put_pixels(var, cx, cy, ms, ms, BLUE); // wall
			else if (c == 'C')
				put_pixels(var, cx, cy, ms, ms, PELLET); // collectible
			else if (c == 'E')
				put_pixels(var, cx, cy, ms, ms, GREEN); // exit
		}

	int px = var->game.Player % WINDOW_WIDTH; // player (yellow)
	int py = var->game.Player / WINDOW_WIDTH;
	put_pixels(var, x0 + px * ms, y0 + py * ms, ms, ms, YELLOW);
	int ghosts[4] = {GHOST_RED, GHOST_PINK, GHOST_CYAN, GHOST_PURPLE};
	for (int i = 0; i < var->game.Enemlen; i++) // enemies (moving)
	{
		int ex = var->game.Enemies[i] % WINDOW_WIDTH;
		int ey = var->game.Enemies[i] / WINDOW_WIDTH;
		put_pixels(var, x0 + ex * ms, y0 + ey * ms, ms, ms, ghosts[i % 4]);
	}
}

int draw(App *var)
{
	for (int i = 0; i < SCREEN_WIDTH; i++)
		for (int j = 0; j < SCREEN_HEIGHT; j++)
		{
			char *dst = var->addr + (j * var->line_length + i * (var->bits_per_pixel / 8));
			*(unsigned int *)dst = BACKGROUND;
		}

	int pacman_x = var->game.Player % WINDOW_WIDTH;
	int pacman_y = var->game.Player / WINDOW_WIDTH;
	static int f = 0;
	f++;
	if (!paused && f % 8 == 0)
	{
		pacman_x += pacman_dx;
		pacman_y += pacman_dy;
		if (pacman_x >= 0 && pacman_x < WINDOW_WIDTH && pacman_y >= 0 && pacman_y < WINDOW_HEIGHT &&
			 var->game.map[pacman_y * WINDOW_WIDTH + pacman_x] != '1' && (var->game.map[pacman_y * WINDOW_WIDTH + pacman_x] != 'E' || var->game.Collectlen == 0))
		{
			int target = pacman_y * WINDOW_WIDTH + pacman_x;
			for (int i = 0; i < var->game.Enemlen; i++)
				if (var->game.Enemies[i] == target) // walked into an enemy
				{
					printf("Enemie did hit you\n");
					exit(0);
				}
			if (var->game.map[pacman_y * WINDOW_WIDTH + pacman_x] == 'C')
			{
				var->game.Collectlen--;
				printf("Good job: %d\n", var->game.Collectlen);
				if (var->game.Collectlen == 0)
					printf("Got all Collectible\n");
			}
			if (var->game.map[pacman_y * WINDOW_WIDTH + pacman_x] == 'E')
			{
				printf("you won!\n");
				exit(0);
			}
			var->game.map[var->game.Player] = '0';
			var->game.Player = pacman_y * WINDOW_WIDTH + pacman_x;
			var->game.map[var->game.Player] = 'P';
		}
		if (f % 32 == 0) // enemies move at half the pacman cadence (50% slower)
			move_enemie(var, var->game.Player % WINDOW_WIDTH, var->game.Player / WINDOW_WIDTH, 0);
	}
	draw_map(var);
	draw_mini_map(var);
	SDL_UpdateTexture(var->texture, NULL, var->pixels, var->line_length);
	SDL_RenderClear(var->renderer);
	SDL_RenderCopy(var->renderer, var->texture, NULL, NULL);
	SDL_RenderPresent(var->renderer);
	return (0);
}

int main(int argc, char **argv)
{
	App var;
	int fd;
	char *str;

	fd = open(argc > 1 ? argv[1] : "maps/001.ber", O_RDONLY);
	var.game = get_map(fd);
	str = get_to_the_target(var.game, fd);
	close(fd);
	if (!str)
		exit(-1);
	free(str);

	SDL_Init(SDL_INIT_VIDEO);
	int win_x = SDL_WINDOWPOS_UNDEFINED;
	int win_y = SDL_WINDOWPOS_UNDEFINED;
	SDL_Rect bounds;
	if (SDL_GetDisplayUsableBounds(0, &bounds) == 0) // fit the window to the visible screen
	{
		SCREEN_WIDTH = bounds.w;
		SCREEN_HEIGHT = bounds.h;
		win_x = bounds.x;
		win_y = bounds.y;
	}
	var.window = SDL_CreateWindow("Pacman", win_x, win_y, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	var.renderer = SDL_CreateRenderer(var.window, -1, SDL_RENDERER_ACCELERATED);
	var.texture = SDL_CreateTexture(var.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	var.pixels = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(Uint32));
	var.addr = (char *)var.pixels;
	var.bits_per_pixel = 32;
	var.line_length = SCREEN_WIDTH * 4;
	var.quit = 0;

	while (!var.quit)
	{
		while (SDL_PollEvent(&var.event))
		{
			if (var.event.type == SDL_QUIT)
				var.quit = 1;
			else if (var.event.type == SDL_KEYDOWN)
			{
				if (var.event.key.keysym.sym == SDLK_ESCAPE)
					var.quit = 1;
				else if (var.event.key.keysym.sym == SDLK_SPACE)
					paused = !paused;
				else
					clicked_key(var.event.key.keysym.sym, &var);
			}
		}
		draw(&var);
	}
	SDL_DestroyTexture(var.texture);
	SDL_DestroyRenderer(var.renderer);
	SDL_DestroyWindow(var.window);
	free(var.pixels);
	SDL_Quit();
	return (0);
}
