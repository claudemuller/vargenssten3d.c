#include <stddef.h>
#include <math.h>
#include "wall.h"
#include "constants.h"
#include "ray.h"
#include "textures.h"

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


