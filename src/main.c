#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include <stdbool.h>
#include <SDL.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

SDL_Window *window;
SDL_Renderer *renderer;

bool is_running;

bool handle_err(void)
{
	fprintf(stderr, "error initialising SDL: %s\n", SDL_GetError());
	return false;
}

bool init_window(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		return handle_err();
	}

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
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

	is_running = true;
	
	return true;
}

void setup(void)
{
	
}

void process_input(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT: {
			is_running = false;
		} break;

		case SDL_KEYDOWN: {
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				is_running = false;
			}
		} break;
	}
}

void render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderClear(renderer);

	SDL_RenderPresent(renderer);
}

void cleanup(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(void)
{
	is_running = false;

	if (!init_window()) {
		return 1;
	}

	setup();

	while (is_running) {
		process_input();
		// update();
		render();
	}

	cleanup();
	
	return 0;
}
