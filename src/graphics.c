#include <SDL.h>
#include "graphics.h"
#include "error.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static uint32_t *colour_buf = NULL;
static SDL_Texture *colour_buf_texture = NULL;

bool init_window(const char *win_title)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		return handle_err();
	}

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window = SDL_CreateWindow(
		win_title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		display_mode.w,
		display_mode.h,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		return handle_err();
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return handle_err();
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	colour_buf = (uint32_t*)malloc(sizeof(uint32_t) * (uint32_t)(WINDOW_WIDTH * WINDOW_HEIGHT));
	colour_buf_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		WINDOW_WIDTH, 
		WINDOW_HEIGHT
	);

	return true;
}

void  clear_colour_buf(const uint32_t colour)
{
	for (size_t i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
		colour_buf[i] = colour;
	}
}

void render_colour_buf(void)
{
	SDL_UpdateTexture(
		colour_buf_texture, 
		NULL, 
		colour_buf, 
		(int)((uint32_t)WINDOW_WIDTH * sizeof(uint32_t))
	);
	SDL_RenderCopy(renderer, colour_buf_texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void draw_pixel(const int x, const int y, const uint32_t colour)
{
	colour_buf[(WINDOW_WIDTH * y) + x] = colour;
}

void free_graphics(void)
{
	free(colour_buf);
	SDL_DestroyTexture(colour_buf_texture);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

