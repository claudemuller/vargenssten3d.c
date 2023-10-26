#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "player.h"

typedef struct {
	player_t player;
	float ticks_last_frame;
	bool is_running;
} game_t;

#endif // GAME_H
