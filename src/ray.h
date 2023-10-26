#ifndef RAY_H
#define RAY_H

#include <stdbool.h>
#include "graphics.h"
#include "player.h"

#define NUM_RAYS WINDOW_WIDTH
#define RAY_FACING_UP 1
#define RAY_FACING_DOWN 1
#define RAY_FACING_LEFT 1
#define RAY_FACING_RIGHT 1

typedef struct {
	float ray_angle;
	float wall_hit_x;
	float wall_hit_y;
	float distance;
	int wall_hit_content;
	bool was_hit_vert;
} ray_t;

extern ray_t rays[NUM_RAYS];

void normalise_angle(float *angle);
float distance_between_points(const float x1, const float y1, const float x2, const float y2);
void cast_ray(const player_t player, float angle, const int strip_id);
void cast_rays(const player_t player);
void render_rays(const player_t player);

#endif // RAY_H
