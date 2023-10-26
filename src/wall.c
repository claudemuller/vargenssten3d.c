#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include "map.h"
#include "wall.h"
#include "constants.h"
#include "ray.h"
#include "textures.h"

// Adjusts the intensity of a colour with an intensity between 0 and 1.
void adjust_colour_intensity(uint32_t *colour, const float factor)
{
	// Extract individual colours and change their intensity
	uint32_t a = (*colour & 0xFF000000);	
	uint32_t r = (*colour & 0x00FF0000) * factor;	
	uint32_t g = (*colour & 0x0000FF00) * factor;	
	uint32_t b = (*colour & 0x000000FF) * factor;	
	// Put it all together again
	*colour = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}

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

		float factor = 200 / perp_distance > 1.0 ? 1.0 : 200 / perp_distance;

		for (int y = 0; y < WINDOW_HEIGHT; y++) {
			// Roof
			if (y < wall_top_pixel) {
				uint32_t colour = 0xFF333333;
				// adjust_colour_intensity(&colour, factor);
				draw_pixel(x, y, colour);
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
				// adjust_colour_intensity(&texel_colour, factor);

				if (rays[x].was_hit_vert) {
					adjust_colour_intensity(&texel_colour, 0.6);
				}

				draw_pixel(x, y, texel_colour);
				continue;
			}

			// Floor
			if (y >= wall_bottom_pixel) {
				uint32_t colour = 0xFF777777;				
				// adjust_colour_intensity(&colour, factor);
				draw_pixel(x, y, colour);
			}
		}
	}	
}


