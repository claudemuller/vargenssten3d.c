#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20
#define MINIMAP_SCALE_FACTOR 0.2

bool map_has_wall_at(const float x, const float y);
int get_map_at(const int i, const int j);
bool is_inside_map(const float x, const float y);
void render_map(void);

#endif // MAP_H
