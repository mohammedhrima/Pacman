#include "header.h"

void set_coors(Coor *coor, int x, int y)
{
    coor->x = x;
    coor->y = y;
    coor->next_x = coor->x * SCALE;
    coor->next_y = coor->y * SCALE;
    coor->real_x = coor->x * SCALE;
    coor->real_y = coor->y * SCALE;
}

Coor *check_coor(Coor *list, int pos, int *len)
{
    if (pos == 0)
    {
        *len = 10;
        return calloc(*len, sizeof(Coor));
    }
    else if (pos + 1 == *len)
    {
        Coor *tmp = calloc(*len * 2, sizeof(Coor));
        memcpy(tmp, list, *len * sizeof(Coor));
        free(list);
        *len *= 2;
        return tmp;
    }
    return list;
}

char *readline(Win *win, int fd)
{
    char c;
    int r = 0;
    char *res = NULL;
    int len = 10;
    int i = 0;
    static int j;
    while ((r = read(fd, &c, 1)) && !win->quit)
    {
        if (res == NULL)
            res = calloc(len, sizeof(char));
        // printf("res <%s\n>", res);
        if (c != '\n')
            res[i++] = c;
        if (i == len)
        {
            char *tmp = calloc(len * 2, sizeof(char));
            strncpy(tmp, res, len);
            free(res);
            res = tmp;
            len *= 2;
        }
        if (c == '0' || c == '1')
            ;
        else if (c == 'P')
            set_coors(&win->player, i - 1, j);
        else if (c == 'E')
            set_coors(&win->exit_, i - 1, j);
        else if (c == 'X')
        {
            win->enemies = check_coor(win->enemies, win->enemies_pos, &win->enemies_len);
            set_coors(&win->enemies[win->enemies_pos++], i - 1, j);
        }
        else if (c == 'C')
        {
            win->collectibles = check_coor(win->collectibles, win->collectibles_pos, &win->collectibles_len);
            set_coors(&win->collectibles[win->collectibles_pos++], i - 1, j);
        }
        else if (c == '\n')
            break;
        else
        {
            free(res);
            printf("Invalid character <%c>\n", c);
            win->quit = true;
        }
    }
    j++;
    return res;
}

char *strjoin(char *left, char *right)
{
    size_t len = 0;
    if (!left && !right)
        return NULL;
    if (left)
        len += strlen(left);
    if (right)
        len += strlen(right);
    char *res = calloc(len + 1, sizeof(char));
    if (left)
        strcpy(res, left);
    if (right)
        strcpy(res + strlen(res), right);
    free(left);
    free(right);
    return res;
}

void print_map(char *msg, char *map, int width, int height)
{
    printf("%s\n", msg);
    for (int i = 0; i < width * height; i++)
    {
        printf("%c", map[i]);
        if ((i + 1) % width == 0)
            printf("\n");
    }
    printf("\n");
}

void parse_map(Win *win, int fd)
{
    int s = 0;
    int e = 0;

    char *line = NULL;
    while ((line = readline(win, fd)) && !win->quit)
    {
        if (!win->map)
            win->map_width = strlen(line);
        if (strlen(line) != win->map_width)
        {
            printf("Invalid map\n");
            win->quit = true;
            break;
        }
        win->map = strjoin(win->map, line);
        win->map_height++;
    }
    win->visited = strdup(win->map);
    if (win->map_height < 3 || win->map_width < 3 || win->player.x == 0 || win->player.y == 0)
    {
        printf("Invalid map\n");
        win->quit = true;
        return;
    }
    print_map("Parse map", win->map, win->map_width, win->map_height);
}

int depth_first_search(Win *win, int x, int y)
{
    static int found;

    if (win->visited[y * win->map_width + x] == 'C')
        found++;
    if (win->visited[y * win->map_width + x] == 'E')
        win->visited[y * win->map_width + x] = 'F';
    else
        win->visited[y * win->map_width + x] = 'P';

    if (found == win->collectibles_pos && win->visited[win->exit_.y * win->map_width + win->exit_.x] == 'F')
        win->map_parsed = true;

    int poses[][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    for (int i = 0; !win->map_parsed && i < sizeof(poses) / sizeof(int); i++)
    {
        int pos = (y + poses[i][0]) * win->map_width + (x + poses[i][1]);
        if (strchr("0CEX", win->visited[pos]) == 0) continue;
        depth_first_search(win, x + poses[i][1], y + poses[i][0]);
    }
    return found;
}

void check_map(Win *win)
{
    if (win->quit)
        return;

    int res = depth_first_search(win, win->player.x, win->player.y);
    print_map("DFS:", win->visited, win->map_width, win->map_height);
    if (win->visited[win->exit_.y * win->map_width + win->exit_.x] != 'F')
    {
        win->quit = true;
        printf("Error in exit\n");
    }
    else if (res != win->collectibles_pos)
    {
        printf("map has %d Collectibles, only %d are reached\n", win->collectibles_pos, res);
        win->quit = true;
    }
}

void move_pacman(Win *win, int x, int y)
{
    int pos = (win->player.y + y) * win->map_width + win->player.x + x;
    char c = win->map[pos];
    // printf("(%d, %d)\n", pos % win->map_width, pos / win->map_width);
    if (strchr("0C", c) || (c == 'E' && win->collectibles_pos == 0) || c == 'X')
    {
        if (c == 'C')
        {
            win->collectibles_pos--;
            printf("found Collectible, %d remaining\n", win->collectibles_pos);
        }
        else if (c == 'E')
        {
            printf("You won !!\n");
            win->quit = true;
        }
        else if (c == 'X')
        {
            printf("Oops You hit the enemy\n");
            win->quit = true;
        }
        win->map[(win->player.y) * win->map_width + win->player.x] = '0';
        win->player.next_x += x * SCALE;
        win->player.next_y += y * SCALE;
        win->player.x += x;
        win->player.y += y;
        win->map[(win->player.y) * win->map_width + win->player.x] = 'P';
    }
}

void handle_events(Win *win)
{
    while (SDL_PollEvent(&win->event) != 0)
    {
        int event = win->event.type;
        if (event == QUIT)
        {
            win->quit = true;
            break;
        }
        else if (event == PRESS)
        {
            int key = win->event.key.keysym.sym;
            if (key == ESC)
                win->quit = true;
            else if (key == UP || key == DOWN || key == LEFT || key == RIGHT)
            {
                win->key = key;
                int x = key == LEFT ? -1 : key == RIGHT ? 1 : 0;
                int y = key == UP ? -1 : key == DOWN ? 1 : 0;
                move_pacman(win, x, y);
            }
        }
    }
}

int pow2(int x)
{
    return x * x;
}

bool is_pacman(Win *win, int x, int y)
{
    bool cond = false;
    int radius = SCALE / 2;
    int x_center = win->player.real_x + radius;
    int y_center = win->player.real_y + radius;
    if (pow2(x_center - x) + pow2(y_center - y) > pow2(radius))
        return false;
    if (win->key == RIGHT || win->key == LEFT)
    {
        if ((win->key == RIGHT && x < x_center) || (win->key == LEFT && x > x_center))
            return true;
        if (abs(y_center - y) >= sin(DEG(win->angle)) * abs(x_center - x))
            return true;
    }
    else if (win->key == DOWN || win->key == UP)
    {
        if ((win->key == DOWN && y < y_center) || (win->key == UP && y > y_center))
            return true;
        if (abs(x_center - x) >= sin(DEG(win->angle)) * abs(y_center - y))
            return true;
    }
    return cond;
}

void put_color(Win *win, int x, int y, uint32_t color)
{
    for (int i = x * SCALE; i < x * SCALE + SCALE; i++)
        for (int j = y * SCALE; j < y * SCALE + SCALE; j++)
            win->pixels[j * win->window_width + i] = color;
}

void draw_pacman(Win *win)
{
    static float eq = SCALE * 0.01;
    win->angle += eq;
    if (win->angle > 42 || win->angle < 0)
        eq = -eq;

    int x_start = win->player.real_x;
    int y_start = win->player.real_y;
    int x_end = win->player.real_x + SCALE;
    int y_end = win->player.real_y + SCALE;
    for (int i = x_start; i < x_end; i++)
    {
        for (int j = y_start; j < y_end; j++)
        {
            uint32_t color = is_pacman(win, i, j) ? YELLOW : BACKGROUND;
            win->pixels[j * win->window_width + i] = color;
        }
    }
#if 0
    win->player.real_x = (win->player.next_x - win->player.real_x) * .05 + win->player.real_x;
    win->player.real_y = (win->player.next_y - win->player.real_y) * .05 + win->player.real_y;
#else
    win->player.real_x = win->player.next_x;
    win->player.real_y = win->player.next_y;
#endif
}

void draw_circle(Win *win, int x_center, int y_center, int radius, int color)
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
                win->pixels[y * win->window_width + x] = color;
            y++;
        }
        x++;
    }
}

void move_enemies(Win *win)
{
    
}

void draw_enemies(Win *win)
{
    static size_t index_move;

    if (index_move % 100 == 0)
    {
        if (index_move == 1000000)
            index_move = 0;
        move_enemies(win);
    }
    index_move++;

    for (int i = 0; i < win->enemies_pos; i++)
    {
        Coor *enemy = &win->enemies[i];
        if (enemy->x == win->player.x && enemy->y == win->player.y)
        {
            printf("enemy hits you\n");
            win->quit = true;
            break;
        }
#if 0
        enemy->real_x = (enemy->next_x - enemy->real_x) * .01 + enemy->real_x;
        enemy->real_y = (enemy->next_y - enemy->real_y) * .01 + enemy->real_y;
#else
        enemy->real_x = enemy->next_x;
        enemy->real_y = enemy->next_y;
#endif
        draw_circle(win, enemy->real_x + SCALE / 2, enemy->real_y + SCALE / 2, SCALE / 2, COLOR1);
        for (int x = enemy->real_x; x < enemy->real_x + SCALE; x++)
            for (int y = enemy->real_y + SCALE / 2; y < enemy->real_y + SCALE; y++)
                win->pixels[y * win->window_width + x] = COLOR1;
    }
}

void draw(Win *win)
{
    // mouth angle
    static float cradius = SCALE / 4;
    static float eq = 0.03;
    if (cradius > SCALE / 4 || cradius < SCALE / 8)
        eq = -eq;
    cradius += eq;

    for (int i = 0; win->map[i]; i++)
    {
        int x = i % win->map_width;
        int y = i / win->map_width;
        switch (win->map[i])
        {
        case '1':
            put_color(win, x, y, BLUE);
            break;
        case 'E':
            put_color(win, x, y, !win->collectibles_pos ? GREEN : RED);
            break;
        case 'C':
            put_color(win, x, y, BACKGROUND);
            draw_circle(win, x * SCALE + SCALE / 2, y * SCALE + SCALE / 2, cradius, ORANGE);
            break;
        default:
            put_color(win, x, y, BACKGROUND);
            break;
        }
    }
    draw_pacman(win);
    draw_enemies(win);
    update_window(win);
}

int main(int argc, char **argv)
{
    Win win = {
        .key = RIGHT,
        .angle = 40,
    };
    int fd = open("map.ber", O_RDONLY);
    if (fd < 0)
    {
        printf("Failed to open map\n");
        exit(1);
    }
    parse_map(&win, fd);
    check_map(&win);
    if (!win.quit)
    {
        print_map("Start game:", win.map, win.map_width, win.map_height);
        init(&win);
        while (!win.quit)
        {
            handle_events(&win);
            draw(&win);
        }

        close_window(&win);
    }
    free(win.map);
    free(win.visited);
    free(win.collectibles);
    free(win.enemies);
}
