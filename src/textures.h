#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>
#include "upng.h"

#define NUM_TEXTURES 9

typedef struct {
    uint32_t *texture_buffer;
    upng_t* upng_texture;
    int width;
    int height;
} texture_t;

extern texture_t wall_textures[NUM_TEXTURES];

void load_wall_textures(void);
void free_wall_textures(void);

#endif // TEXTURES_H
