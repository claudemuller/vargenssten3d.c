#include <math.h>
#include <float.h>
#include <stddef.h>
#include "ray.h"
#include "player.h"
#include "map.h"
#include "constants.h"

ray_t rays[NUM_RAYS] = {0};

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

void cast_ray(const player_t player, const float angle, const int strip_id) {
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

    rays[strip_id].ray_angle = ray_angle;

    if (vert_hit_distance < horz_hit_distance) {
        rays[strip_id].distance = vert_hit_distance;
        rays[strip_id].wall_hit_x = vert_wall_hit_x;
        rays[strip_id].wall_hit_y = vert_wall_hit_y;
        rays[strip_id].wall_hit_content = vert_wall_content;
        rays[strip_id].was_hit_vert = true;
    } else {
        rays[strip_id].distance = horz_hit_distance;
        rays[strip_id].wall_hit_x = horz_wall_hit_x;
        rays[strip_id].wall_hit_y = horz_wall_hit_y;
        rays[strip_id].wall_hit_content = horz_wall_content;
        rays[strip_id].was_hit_vert = false;
    }
}

void cast_rays(const player_t player)
{
	for (size_t col = 0; col < NUM_RAYS; col++) {
		float ray_angle = player.rotation_angle + atan((col - NUM_RAYS / 2.0) / DISTANCE_PROJECTION_PLANE);
		cast_ray(player, ray_angle, col);
	}
}

void render_rays(const player_t player)
{
	for (size_t i = 0; i < NUM_RAYS; i++) {
		draw_line(
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			rays[i].wall_hit_x * MINIMAP_SCALE_FACTOR,
			rays[i].wall_hit_y * MINIMAP_SCALE_FACTOR,
			0xFF0000FF
		);
	}
}

