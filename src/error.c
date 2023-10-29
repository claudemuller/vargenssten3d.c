#include "error.h"

bool handle_err(void)
{
    fprintf(stderr, "error initialising SDL: %s\n", SDL_GetError());
    return false;
}
