#include <math.h>
#include <stddef.h>
#include "map.h"
#include "constants.h"
#include "graphics.h"

static const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 9, 0, 6, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5}
};

bool map_has_wall_at(const float x, const float y)
{
	if (
		x < 0 || x > MAP_NUM_COLS * TILE_SIZE 
		|| y < 0 || y > MAP_NUM_ROWS * TILE_SIZE
	) {
		return true;
	}

	const int col = floor(x / TILE_SIZE);
	const int row = floor(y / TILE_SIZE);

	return map[row][col] != 0;
}

int get_map_at(const int i, const int j)
{
	return map[i][j];	
}

bool is_inside_map(const float x, const float y)
{
	return x >= 0 && x <= MAP_NUM_COLS * TILE_SIZE && y >= 0 && y <= MAP_NUM_ROWS * TILE_SIZE;	
}

void render_map(void)
{
	int tile_x = 0;
	int tile_y = 0;
	uint32_t tile_colour = 0;

	for (size_t i = 0; i < MAP_NUM_ROWS; i++) {
		for (size_t j = 0; j < MAP_NUM_COLS; j++) {
			tile_x = j * TILE_SIZE;
			tile_y = i * TILE_SIZE;
			tile_colour = map[i][j] != 0 ? 0xFFFFFFFF : 0xFF000000;

			draw_rect(
				tile_x * MINIMAP_SCALE_FACTOR,
				tile_y * MINIMAP_SCALE_FACTOR,
				ceil(TILE_SIZE * MINIMAP_SCALE_FACTOR),
				ceil(TILE_SIZE * MINIMAP_SCALE_FACTOR),
				tile_colour
			);
		}
	}
}

