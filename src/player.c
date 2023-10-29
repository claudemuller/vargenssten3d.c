#include "constants.h"
#include "game.h"
#include "graphics.h"
#include "map.h"
#include "player.h"
#include <math.h>

void move_player(player_t* player, float delta_time)
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

void render_player(player_t* player)
{
    draw_rect(
        player->x * MINIMAP_SCALE_FACTOR,
        player->y * MINIMAP_SCALE_FACTOR,
        player->height * MINIMAP_SCALE_FACTOR,
        player->width * MINIMAP_SCALE_FACTOR,
        0xFFFFFFFF);
}
