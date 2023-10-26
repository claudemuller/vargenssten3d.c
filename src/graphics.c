#include <SDL.h>
#include <stdint.h>
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

void draw_line(const int x0, const int y0, const int x1, const int y1, const uint32_t colour)
{
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);
	int side_len = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);
	float x_inc = delta_x / (float)side_len;
	float y_inc = delta_y / (float)side_len;
	float cur_x = x0;
	float cur_y = y0;

	for (int i = 0; i <= side_len; i++) {
		draw_pixel(round(cur_x), round(cur_y), colour);
		cur_x += x_inc;
		cur_y += y_inc;
	}
}

void draw_rect(const int x, const int y, const int w, const int h, const uint32_t colour)
{
	// TODO: better way to loop?
	for (int i = y; i < y+h; i++) {
		for (int j = x; j < x+w; j++) {
			draw_pixel(j, i, colour);
		}
	}
}

void free_graphics(void)
{
	free(colour_buf);
	SDL_DestroyTexture(colour_buf_texture);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

