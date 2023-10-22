#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include <stdbool.h>
#include <SDL.h>

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

SDL_Window *window;
SDL_Renderer *renderer;

bool is_running;

float ticks_last_frame = 0;
int player_x, player_y;

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
		"Vargenssten 3D",
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
	player_x = 0;
	player_y = 0;
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

void update(void)
{
	int time_to_wait = FRAME_TIME_LEN - (SDL_GetTicks() - ticks_last_frame);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LEN) {
		SDL_Delay(time_to_wait);
	}

	float delta_time = (SDL_GetTicks() - ticks_last_frame) / 1000.0f;
	ticks_last_frame = SDL_GetTicks();

	player_x += 50 * delta_time;
	player_y += 50 * delta_time;
}

void render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_Rect rect = (SDL_Rect){
		.x = player_x,
		.y = player_y,
		.w = 20,
		.h = 20
	};
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	SDL_RenderFillRect(renderer, &rect);
	
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
		update();
		render();
	}

	cleanup();
	
	return 0;
}
