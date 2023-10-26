#ifndef WALL_H
#define WALL_H

#include <stdint.h>
#include "player.h"

void change_colour_intensity(uint32_t *colour, const float factor);
void render_wall_projection(const player_t player);

#endif // WALL_H

