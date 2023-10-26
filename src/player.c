#include <math.h>
#include "player.h"
#include "map.h"
#include "game.h"
#include "graphics.h"
#include "constants.h"

// player_t player = {
// 	.x = MAP_NUM_COLS * TILE_SIZE / 2.0,
// 	.y = MAP_NUM_ROWS * TILE_SIZE / 2.0,
// 	.width = 1,
// 	.height = 1,
// 	.turn_direction = 0,
// 	.walk_direction = 0,
// 	.rotation_angle = PI / 2,
// 	.walk_speed = 100,
// 	.turn_speed = 45 * (PI / 180),
// };

void move_player(player_t *player, float delta_time)
{
	player->rotation_angle += player->turn_direction * player->turn_speed * delta_time;
	const float move_step = player->walk_direction * player->walk_speed * delta_time;
	const float new_x = player->x + cos(player->rotation_angle) * move_step;
	const float new_y = player->y + sin(player->rotation_angle) * move_step;

	if (!map_has_wall_at(new_x, new_y)) {
		player->x = new_x;
		player->y = new_y;
	}
}

void render_player(player_t *player)
{
	draw_rect(
		player->x * MINIMAP_SCALE_FACTOR,
		player->y * MINIMAP_SCALE_FACTOR,
		player->height * MINIMAP_SCALE_FACTOR,
		player->width * MINIMAP_SCALE_FACTOR,
		0xFFFFFFFF
	);

	// const int look_indicator_len = 40;
	// SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	// SDL_RenderDrawLine(
	// 	renderer,
	// 	player_rect.x,
	// 	player_rect.y,
	// 	player_rect.x + cos(player.rotation_angle) * look_indicator_len,
	// 	player_rect.y + sin(player.rotation_angle) * look_indicator_len
	// );
}

