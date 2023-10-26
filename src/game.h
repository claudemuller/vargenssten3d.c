#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "player.h"

enum input_method {
	INPUT_MOUSE,
	INPUT_KEYBOARD
};

typedef struct {
	player_t player;
	float ticks_last_frame;
	enum input_method input_method;
	bool is_running;
} game_t;

#endif // GAME_H
