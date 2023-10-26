#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>
#include <SDL.h>

bool handle_err(void)
{
	fprintf(stderr, "error initialising SDL: %s\n", SDL_GetError());
	return false;
}

#endif // ERROR_H
