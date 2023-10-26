#include <stdbool.h>
#include <SDL.h>
#include "constants.h"
#include "graphics.h"
#include "game.h"
#include "map.h"
#include "ray.h"
#include "player.h"
#include "textures.h"

texture_t wall_textures[NUM_TEXTURES] = {0}; 

void render_wall_projection(const player_t player)
{
	float perp_distance = 0.0;
	float projected_wall_height = 0.0;
	int wall_strip_height = 0.0;

	for (size_t x = 0; x < NUM_RAYS; x++) {
		perp_distance = rays[x].distance * cos(rays[x].ray_angle - player.rotation_angle);
		projected_wall_height = (TILE_SIZE / perp_distance) * DISTANCE_PROJECTION_PLANE;
		wall_strip_height = (int)projected_wall_height;

		int wall_top_pixel = (WINDOW_HEIGHT / 2) - (wall_strip_height / 2);
		wall_top_pixel = wall_top_pixel < 0 ? 0 : wall_top_pixel;
		int wall_bottom_pixel = (WINDOW_HEIGHT / 2) + (wall_strip_height / 2);
		wall_bottom_pixel = wall_bottom_pixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wall_bottom_pixel;

		for (int y = 0; y < WINDOW_HEIGHT; y++) {
			// Roof
			if (y < wall_top_pixel) {
				draw_pixel(x, y, 0xFF333333);
				continue;
			}

			// Walls
			size_t texture_offset_x;
			if (rays[x].was_hit_vert) {
				texture_offset_x = (int)rays[x].wall_hit_y % TILE_SIZE;
			} else {
				texture_offset_x = (int)rays[x].wall_hit_x % TILE_SIZE;
			}

			const size_t tex_id = rays[x].wall_hit_content-1;

			int texture_width = wall_textures[tex_id].width;
			int texture_height = wall_textures[tex_id].height;

			if (y >= wall_top_pixel && y < wall_bottom_pixel) {
				const int distance_from_top = y + (wall_strip_height / 2) - (WINDOW_HEIGHT / 2);
				const size_t texture_offset_y = distance_from_top * ((float)texture_width / wall_strip_height);
				uint32_t texel_colour = wall_textures[tex_id].texture_buffer[(texture_height * texture_offset_y) + texture_offset_x];
				// texel_colour = (texel_colour & 0xfefefe) >> 1;
				draw_pixel(x, y, texel_colour);
				continue;
			}

			// Floor
			if (y >= wall_bottom_pixel) {
				draw_pixel(x, y, 0xFF777777);
			}
		}
	}	
}

void setup(void)
{
	load_wall_textures();
}

void process_input(game_t *game)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT: {
			game->is_running = false;
		} break;

		case SDL_KEYDOWN: {
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				game->is_running = false;
			}
			if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
				game->player.walk_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
				game->player.walk_direction = -1;
			}
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
				game->player.turn_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
				game->player.turn_direction = -1;
			}
		} break;

		case SDL_KEYUP: {
			if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
				game->player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
				game->player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_d) {
				game->player.turn_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_a) {
				game->player.turn_direction = 0;
			}
		} break;
	}
}

void update(game_t *game)
{
	const int time_to_wait = FRAME_TIME_LEN - (SDL_GetTicks() - game->ticks_last_frame);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LEN) {
		SDL_Delay(time_to_wait);
	}

	const float delta_time = (SDL_GetTicks() - game->ticks_last_frame) / 1000.0f;
	game->ticks_last_frame = SDL_GetTicks();

	move_player(&(game->player), delta_time);
	cast_rays(game->player);
}

void render(game_t *game)
{
	clear_colour_buf(0xFF000000);
	render_wall_projection(game->player);

	// Render minimap
	render_map();
	render_rays();
	render_player(&(game->player));

	render_colour_buf();
}

void cleanup(void)
{
	free_wall_textures();
	free_graphics();
}

int main(void)
{
	game_t game = {
		.is_running = false,
		.ticks_last_frame = 0.0,
		.player = {
			.x = MAP_NUM_COLS * TILE_SIZE / 2.0,
			.y = MAP_NUM_ROWS * TILE_SIZE / 2.0,
			.width = 1,
			.height = 1,
			.turn_direction = 0,
			.walk_direction = 0,
			.rotation_angle = PI / 2,
			.walk_speed = 100,
			.turn_speed = 45 * (PI / 180),
		}
	};

	if (!(game.is_running = init_window("Vargenssten 3D"))) {
		return 1;
	}

	setup();

	while (game.is_running) {
		process_input(&game);
		update(&game);
		render(&game);
	}

	cleanup();
	
	return 0;
}
