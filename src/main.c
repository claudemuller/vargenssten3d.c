#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <SDL.h>
#include "upng.h"
#include "constants.h"
#include "graphics.h"
#include "map.h"
#include "textures.h"

struct player_t {
	float x;
	float y;
	float width;
	float height;
	float rotation_angle;
	float walk_speed;
	float turn_speed;
	int turn_direction; // -1 for left; +1 for right
	int walk_direction; // -1 for back; +1 for front
} player;

struct ray_t {
	float ray_angle;
	float wall_hit_x;
	float wall_hit_y;
	float distance;
	int wall_hit_content;
	bool was_hit_vert;
} rays[NUM_RAYS];

texture_t wall_textures[NUM_TEXTURES] = {0}; 

bool is_running = false;
float ticks_last_frame = 0;

void setup(void)
{
	player.x = MAP_NUM_COLS * TILE_SIZE / 2.0;
	player.y = MAP_NUM_ROWS * TILE_SIZE / 2.0;
	player.width = 1;
	player.height = 1;
	player.turn_direction = 0;
	player.walk_direction = 0;
	player.rotation_angle = PI / 2;
	player.walk_speed = 100;
	player.turn_speed = 45 * (PI / 180);

	load_wall_textures();
}

float normalise_angle(const float angle)
{
	float norm_angle = remainder(angle, TWO_PI);
	if (norm_angle < 0) {
		norm_angle = TWO_PI + norm_angle;
	}
	return norm_angle;
}

float distance_between_points(const float x1, const float y1, const float x2, const float y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

void cast_ray(const float angle, const int stripId) {
    float ray_angle = normalise_angle(angle);
    
    bool is_ray_facing_down = ray_angle > 0 && ray_angle < PI;
    bool is_ray_facing_up = !is_ray_facing_down;

	bool is_ray_facing_right = ray_angle < 0.5 * PI || ray_angle > 1.5 * PI;
    bool is_ray_facing_left = !is_ray_facing_right;
    
    float xintercept, yintercept;
    float xstep, ystep;

	// Horizontal intercept
    int found_horz_wall_hit = false;
    float horz_wall_hit_x = 0;
    float horz_wall_hit_y = 0;
    int horz_wall_content = 0;

    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += is_ray_facing_down ? TILE_SIZE : 0;

    xintercept = player.x + (yintercept - player.y) / tan(ray_angle);

    ystep = TILE_SIZE;
    ystep *= is_ray_facing_up ? -1 : 1;

    xstep = TILE_SIZE / tan(ray_angle);
    xstep *= (is_ray_facing_left && xstep > 0) ? -1 : 1;
    xstep *= (is_ray_facing_right && xstep < 0) ? -1 : 1;

    float next_horz_touch_x = xintercept;
    float next_horz_touch_y = yintercept;

    while (is_inside_map(next_horz_touch_x, next_horz_touch_y)) {
        float x_to_check = next_horz_touch_x;
        float y_to_check = next_horz_touch_y + (is_ray_facing_up ? -1 : 0);
        
        if (map_has_wall_at(x_to_check, y_to_check)) {
            horz_wall_hit_x = next_horz_touch_x;
            horz_wall_hit_y = next_horz_touch_y;
            horz_wall_content = get_map_at(
				(int)floor(y_to_check / TILE_SIZE),
				 (int)floor(x_to_check / TILE_SIZE)
			);
            found_horz_wall_hit = true;
            break;
        }

        next_horz_touch_x += xstep;
        next_horz_touch_y += ystep;
    }
    
	// Vertical intercept
    int found_vert_wall_hit = false;
    float vert_wall_hit_x = 0;
    float vert_wall_hit_y = 0;
    int vert_wall_content = 0;

    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += is_ray_facing_right ? TILE_SIZE : 0;

    yintercept = player.y + (xintercept - player.x) * tan(ray_angle);

    xstep = TILE_SIZE;
    xstep *= is_ray_facing_left ? -1 : 1;

    ystep = TILE_SIZE * tan(ray_angle);
    ystep *= (is_ray_facing_up && ystep > 0) ? -1 : 1;
    ystep *= (is_ray_facing_down && ystep < 0) ? -1 : 1;

    float next_vert_touch_x = xintercept;
    float next_vert_touch_y = yintercept;

    while (is_inside_map(next_vert_touch_x, next_vert_touch_y)) {
        float x_to_check = next_vert_touch_x + (is_ray_facing_left ? -1 : 0);
        float y_to_check = next_vert_touch_y;
        
        if (map_has_wall_at(x_to_check, y_to_check)) {
            vert_wall_hit_x = next_vert_touch_x;
            vert_wall_hit_y = next_vert_touch_y;
            vert_wall_content = get_map_at(
				(int)floor(y_to_check / TILE_SIZE),
				(int)floor(x_to_check / TILE_SIZE)
			);
            found_vert_wall_hit = true;
            break;
        }

        next_vert_touch_x += xstep;
        next_vert_touch_y += ystep;
    }

    float horz_hit_distance = found_horz_wall_hit
        ? distance_between_points(player.x, player.y, horz_wall_hit_x, horz_wall_hit_y)
        : FLT_MAX;
    float vert_hit_distance = found_vert_wall_hit
        ? distance_between_points(player.x, player.y, vert_wall_hit_x, vert_wall_hit_y)
        : FLT_MAX;

    rays[stripId].ray_angle = ray_angle;

    if (vert_hit_distance < horz_hit_distance) {
        rays[stripId].distance = vert_hit_distance;
        rays[stripId].wall_hit_x = vert_wall_hit_x;
        rays[stripId].wall_hit_y = vert_wall_hit_y;
        rays[stripId].wall_hit_content = vert_wall_content;
        rays[stripId].was_hit_vert = true;
    } else {
        rays[stripId].distance = horz_hit_distance;
        rays[stripId].wall_hit_x = horz_wall_hit_x;
        rays[stripId].wall_hit_y = horz_wall_hit_y;
        rays[stripId].wall_hit_content = horz_wall_content;
        rays[stripId].was_hit_vert = false;
    }
}

void cast_rays(void)
{
	for (size_t col = 0; col < NUM_RAYS; col++) {
		float ray_angle = player.rotation_angle + atan((col - NUM_RAYS / 2.0) / DISTANCE_PROJECTION_PLANE);
		cast_ray(ray_angle, col);
	}
}

void render_rays(void)
{
	/*
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	for (size_t i = 0; i < NUM_RAYS; i++) {
		SDL_RenderDrawLine(
			renderer,
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			rays[i].wall_hit_x * MINIMAP_SCALE_FACTOR,
			rays[i].wall_hit_y * MINIMAP_SCALE_FACTOR
		);
	}
	*/
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
	/*
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	const SDL_Rect player_rect = {
		.x = player.x * MINIMAP_SCALE_FACTOR,
		.y = player.y * MINIMAP_SCALE_FACTOR,
		.h = player.height * MINIMAP_SCALE_FACTOR,
		.w = player.width * MINIMAP_SCALE_FACTOR
	};
	SDL_RenderFillRect(renderer, &player_rect);

	const int look_indicator_len = 40;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawLine(
		renderer,
		player_rect.x,
		player_rect.y,
		player_rect.x + cos(player.rotation_angle) * look_indicator_len,
		player_rect.y + sin(player.rotation_angle) * look_indicator_len
	);
	*/
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
			if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
				player.walk_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
				player.walk_direction = -1;
			}
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
				player.turn_direction = 1;
			}
			if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
				player.turn_direction = -1;
			}
		} break;

		case SDL_KEYUP: {
			if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
				player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
				player.walk_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_d) {
				player.turn_direction = 0;
			}
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_a) {
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
	cast_rays();
}

void render_wall_projection(void)
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

void render(void)
{
	clear_colour_buf(0xFF000000);
	render_wall_projection();

	// Render minimap
	render_map();
	// render_rays();
	// render_player();

	render_colour_buf();
}

void cleanup(void)
{
	free_wall_textures();
	free_graphics();
}

int main(void)
{
	if (!(is_running = init_window("Vargenssten 3D"))) {
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
