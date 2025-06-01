#include "header.h"

void init(Win *win)
{
    win->window_height = SCALE * win->map_height;
    win->window_width = SCALE * win->map_width;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }
    win->window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   win->window_width, win->window_height, SDL_WINDOW_SHOWN);
    if (win->window == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
    win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED);
    if (win->renderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win->window);
        SDL_Quit();
        return;
    }
    win->texture = SDL_CreateTexture(win->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_PIXELFORMAT_ARGB8888,
                                     win->window_width, win->window_height);
    if (win->texture == NULL)
    {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(win->renderer);
        SDL_DestroyWindow(win->window);
        SDL_Quit();
        return;
    }
    win->pixels = (Uint32 *)calloc(win->window_width * win->window_height, sizeof(Uint32));
    if (win->pixels == NULL)
    {
        printf("Failed to allocate pixel buffer\n");
        SDL_DestroyTexture(win->texture);
        SDL_DestroyRenderer(win->renderer);
        SDL_DestroyWindow(win->window);
        SDL_Quit();
        return;
    }
}

void close_window(Win *win)
{
    free(win->pixels);
    SDL_DestroyTexture(win->texture);
    SDL_DestroyRenderer(win->renderer);
    SDL_DestroyWindow(win->window);
    SDL_Quit();
}

void update_window(Win *win)
{
    // Update texture with the pixel buffer
    SDL_UpdateTexture(win->texture, NULL, win->pixels, win->window_width * sizeof(Uint32));
    SDL_RenderClear(win->renderer);                          // Clear the screen
    SDL_RenderCopy(win->renderer, win->texture, NULL, NULL); // Render the texture
    SDL_RenderPresent(win->renderer);                        // Update the screen
}