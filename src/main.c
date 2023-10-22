#include <stdbool.h>
#include <limits.h>
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

struct ray_t {
	float ray_angle;
	float wall_hit_x;
	float wall_hit_y;
	bool was_hit_vert;
	float distance;
	// Use bitmap?
	bool is_ray_facing_up;
	bool is_ray_facing_down;
	bool is_ray_facing_left;
	bool is_ray_facing_right;
	int wall_hit_content;
} rays[NUM_RAYS];

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

float normalise_angle(const float angle)
{
	float norm_angle = remainder(angle, TWO_PI);
	if (angle < 0) {
		norm_angle = TWO_PI + angle;
	}
	return norm_angle;
}

float distance_between_points(const int x1, const int y1, const int x2, const int y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
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

void cast_ray(const float angle, const int strip_id)
{
	const float ray_angle = normalise_angle(angle);

	bool is_ray_facing_down = ray_angle > 0 && ray_angle < PI;
	bool is_ray_facing_up = !is_ray_facing_down;
	bool is_ray_facing_right = ray_angle < 0.5 * PI || ray_angle > 1.5 * PI;
	bool is_ray_facing_left = !is_ray_facing_right;
	
	// Horizontal intercept
	float y_intercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	y_intercept += is_ray_facing_down ? TILE_SIZE : 0;

	float x_intercept = player.x + (y_intercept - player.y) / tan(ray_angle);
		
	float ystep = TILE_SIZE;
	ystep *= is_ray_facing_up ? -1 : 1;

	float xstep = TILE_SIZE / tan(ray_angle);
	xstep *= is_ray_facing_left && xstep > 0 ? -1 : 1;
	xstep *= is_ray_facing_right && xstep < 0 ? -1 : 1;

	float next_horz_touch_x = x_intercept;
	float next_horz_touch_y = y_intercept;
	bool found_horz_wall_hit = false;
	float horz_wall_hit_x = 0;
	float horz_wall_hit_y = 0;
	int horz_wall_content = 0;
		
	while ( 
		next_horz_touch_x >= 0 && next_horz_touch_x <= MAP_NUM_COLS * TILE_SIZE
		&& next_horz_touch_y >= 0 && next_horz_touch_y <= MAP_NUM_ROWS * TILE_SIZE
	) {
		float x_to_check = next_horz_touch_x;
		float y_to_check = next_horz_touch_y + (is_ray_facing_up ? -1 : 0);
		
		if (map_has_wall_at(x_to_check, y_to_check)) {
			found_horz_wall_hit = true;
			horz_wall_hit_x = next_horz_touch_x;
			horz_wall_hit_y = next_horz_touch_y;
			horz_wall_content = map[(int)floor(y_to_check / TILE_SIZE)][(int)floor(x_to_check / TILE_SIZE)];
			break;
		}

		next_horz_touch_x += xstep;
		next_horz_touch_y += ystep;
	}

	// Vertical intercept
	x_intercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	x_intercept += is_ray_facing_right ? TILE_SIZE : 0;

	y_intercept = player.y + (x_intercept - player.x) / tan(ray_angle);
		
	xstep = TILE_SIZE;
	xstep *= is_ray_facing_left ? -1 : 1;

	ystep = TILE_SIZE / tan(ray_angle);
	ystep *= is_ray_facing_up && ystep > 0 ? -1 : 1;
	ystep *= is_ray_facing_down && ystep < 0 ? -1 : 1;

	float next_vert_touch_x = x_intercept;
	float next_vert_touch_y = y_intercept;
	bool found_vert_wall_hit = false;
	float vert_wall_hit_x = 0;
	float vert_wall_hit_y = 0;
	int vert_wall_content = 0;
		
	while ( 
		next_vert_touch_x >= 0 && next_vert_touch_x <= MAP_NUM_COLS * TILE_SIZE
		&& next_vert_touch_y >= 0 && next_vert_touch_y <= MAP_NUM_ROWS * TILE_SIZE
	) {
		float x_to_check = next_vert_touch_x + (is_ray_facing_left ? -1 : 0);
		float y_to_check = next_vert_touch_y;
		
		if (map_has_wall_at(x_to_check, y_to_check)) {
			found_vert_wall_hit = true;
			vert_wall_hit_x = next_vert_touch_x;
			vert_wall_hit_y = next_vert_touch_y;
			vert_wall_content = map[(int)floor(y_to_check / TILE_SIZE)][(int)floor(x_to_check / TILE_SIZE)];
			break;
		}

		next_vert_touch_x += xstep;
		next_vert_touch_y += ystep;
	}

	const float horz_hit_distance = found_horz_wall_hit
		? distance_between_points(player.x, player.y, horz_wall_hit_x, horz_wall_hit_y)
		: INT_MAX;
	const float vert_hit_distance = found_vert_wall_hit
		? distance_between_points(player.x, player.y, horz_wall_hit_x, horz_wall_hit_y)
		: INT_MAX;

	rays[strip_id].ray_angle = ray_angle;
	rays[strip_id].is_ray_facing_down = is_ray_facing_down;
	rays[strip_id].is_ray_facing_up = is_ray_facing_up;
	rays[strip_id].is_ray_facing_left = is_ray_facing_left;
	rays[strip_id].is_ray_facing_right = is_ray_facing_right;
	
	if (vert_hit_distance < horz_hit_distance) {
		rays[strip_id].distance = vert_hit_distance;
		rays[strip_id].wall_hit_x = vert_wall_hit_x;
		rays[strip_id].wall_hit_y = vert_wall_hit_y;
		rays[strip_id].wall_hit_content = vert_wall_content;
		rays[strip_id].was_hit_vert = true;
		return;
	}

	rays[strip_id].distance = horz_hit_distance;
	rays[strip_id].wall_hit_x = horz_wall_hit_x;
	rays[strip_id].wall_hit_y = horz_wall_hit_y;
	rays[strip_id].wall_hit_content = horz_wall_content;
	rays[strip_id].was_hit_vert = false;
}

void cast_rays(void)
{
	float ray_angle = player.rotation_angle - (FOV_ANGLE / 2);

	for (size_t strip_id = 0; strip_id < NUM_RAYS; strip_id++) {
		cast_ray(ray_angle, strip_id);
		ray_angle += FOV_ANGLE / NUM_RAYS;
	}
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
	cast_rays();
}

void render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	render_map();
	// render_rays();
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
