#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include <stdbool.h>
#include <SDL.h>

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)

#define PI 3.14159265
#define TWO_PI 6.28318530

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define FOV_ANGLE (60 * PI / 180)
#define NUM_RAYS WINDOW_WIDTH
#define MINIMAP_SCALE_FACTOR 1.0

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct player_t {
	float x;
	float y;
	float width;
	float height;
	int turn_direction; // -1 for left; +1 for right
	int walk_direction; // -1 for back; +1 for front
	float rotation_angle;
	float walk_speed;
	float turn_speed;
} player;

SDL_Window *window;
SDL_Renderer *renderer;

bool is_running;

float ticks_last_frame = 0;

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
	player.x = WINDOW_WIDTH / 2.0;
	player.y = WINDOW_HEIGHT / 2.0;
	player.width = 1;
	player.height = 1;
	player.turn_direction = 0;
	player.walk_direction = 0;
	player.rotation_angle = PI / 2;
	player.walk_speed = 100;
	player.turn_speed = 45 * (PI / 180);
}

void render_map(void)
{
	for (size_t i = 0; i < MAP_NUM_ROWS; i++) {
		for (size_t j = 0; j < MAP_NUM_COLS; j++) {
			const int tile_x = j * TILE_SIZE;
			const int tile_y = i * TILE_SIZE;
			const int tile_colour = map[i][j] != 0 ? 255 : 0;

			SDL_SetRenderDrawColor(renderer, tile_colour, tile_colour, tile_colour, 255);
			const SDL_Rect map_tile_rect = {
				.x = tile_x * MINIMAP_SCALE_FACTOR,
				.y = tile_y * MINIMAP_SCALE_FACTOR,
				.w = TILE_SIZE * MINIMAP_SCALE_FACTOR,
				.h = TILE_SIZE * MINIMAP_SCALE_FACTOR
			};
			SDL_RenderFillRect(renderer, &map_tile_rect);
		}
	}
}

bool map_has_wall_at(const float x, const float y)
{
	if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
		return true;
	}

	const int col = floor(x / TILE_SIZE);
	const int row = floor(y / TILE_SIZE);

	return map[row][col] != 0;
}

void move_player(float delta_time)
{
	player.rotation_angle += player.turn_direction * player.turn_speed * delta_time;
	const float move_step = player.walk_direction * player.walk_speed * delta_time;
	const float new_x = player.x + cos(player.rotation_angle) * move_step;
	const float new_y = player.y + sin(player.rotation_angle) * move_step;

	if (!map_has_wall_at(new_x, new_y)) {
		player.x = new_x;
		player.y = new_y;
	}
}

void render_player(void)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	const SDL_Rect player_rect = {
		.x = player.x * MINIMAP_SCALE_FACTOR,
		.y = player.y * MINIMAP_SCALE_FACTOR,
		.h = player.height * MINIMAP_SCALE_FACTOR,
		.w = player.width * MINIMAP_SCALE_FACTOR
	};
	SDL_RenderFillRect(renderer, &player_rect);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderDrawLine(
		renderer,
		player.x * MINIMAP_SCALE_FACTOR,
		player.y * MINIMAP_SCALE_FACTOR,
		player.x + cos(player.rotation_angle) * 40 * MINIMAP_SCALE_FACTOR,
		player.y + sin(player.rotation_angle) * 40 * MINIMAP_SCALE_FACTOR
	);
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
			if (event.key.keysym.sym == SDLK_UP) {
				player.walk_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				player.walk_direction = -1;
			}
			if (event.key.keysym.sym == SDLK_RIGHT) {
				player.turn_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_LEFT) {
				player.turn_direction = -1;
			}
		} break;

		case SDL_KEYUP: {
			if (event.key.keysym.sym == SDLK_UP) {
				player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_LEFT) {
				player.turn_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_RIGHT) {
				player.turn_direction = 0;
			}
		} break;
	}
}

void update(void)
{
	const int time_to_wait = FRAME_TIME_LEN - (SDL_GetTicks() - ticks_last_frame);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LEN) {
		SDL_Delay(time_to_wait);
	}

	const float delta_time = (SDL_GetTicks() - ticks_last_frame) / 1000.0f;
	ticks_last_frame = SDL_GetTicks();

	move_player(delta_time);
}

void render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	render_map();
	render_player();
	
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
