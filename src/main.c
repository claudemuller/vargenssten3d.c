#include "constants.h"
#include "game.h"
#include "graphics.h"
#include "map.h"
#include "player.h"
#include "ray.h"
#include "textures.h"
#include "wall.h"
#include "SDL.h"
#include <stdbool.h>
#include <stdlib.h>

texture_t wall_textures[NUM_TEXTURES] = { 0 };

void setup(void);
void process_input(game_t* game);
void update(game_t* game);
void render(game_t* game);
void render_wall_projection(const player_t player);
void cleanup(void);

int main(void)
{
    game_t game = { .is_running = false,
        .ticks_last_frame = 0.0,
        .player = {
            .x = MAP_NUM_COLS * TILE_SIZE / 2.0,
            .y = MAP_NUM_ROWS * TILE_SIZE / 2.0,
            .width = 1,
            .height = 1,
            .turn_direction = 0,
            .walk_direction = 0,
            .rotation_angle = PI / 2,
            .walk_speed = 100,
            .turn_speed = 75 * (PI / 180),
        } };

    if (!(game.is_running = init_window("Vargenssten 3D"))) {
        return EXIT_FAILURE;
    }

    setup();

    while (game.is_running) {
        process_input(&game);
        update(&game);
        render(&game);
    }

    cleanup();

    return EXIT_SUCCESS;
}

void setup(void)
{
    load_wall_textures();
}

bool mouse_moving = false;

void process_input(game_t* game)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT: {
        game->is_running = false;
    } break;

    case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            game->is_running = false;
        }
        if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
            game->player.walk_direction = 1;
        }
        if (event.key.keysym.sym == SDLK_DOWN
            || event.key.keysym.sym == SDLK_s) {
            game->player.walk_direction = -1;
        }
        if (event.key.keysym.sym == SDLK_RIGHT
            || event.key.keysym.sym == SDLK_d) {
            game->player.turn_direction = 1;
        }
        if (event.key.keysym.sym == SDLK_LEFT
            || event.key.keysym.sym == SDLK_a) {
            game->player.turn_direction = -1;
        }
    } break;

    case SDL_KEYUP: {
        if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
            game->player.walk_direction = 0;
        }
        if (event.key.keysym.sym == SDLK_DOWN
            || event.key.keysym.sym == SDLK_s) {
            game->player.walk_direction = 0;
        }
        if (event.key.keysym.sym == SDLK_LEFT
            || event.key.keysym.sym == SDLK_d) {
            game->player.turn_direction = 0;
        }
        if (event.key.keysym.sym == SDLK_RIGHT
            || event.key.keysym.sym == SDLK_a) {
            game->player.turn_direction = 0;
        }
    } break;
    }
}

void update(game_t* game)
{
    const int time_to_wait
        = FRAME_TIME_LEN - (SDL_GetTicks() - game->ticks_last_frame);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LEN) {
        SDL_Delay(time_to_wait);
    }

    const float delta_time
        = (SDL_GetTicks() - game->ticks_last_frame) / 1000.0f;
    game->ticks_last_frame = SDL_GetTicks();

    move_player(&(game->player), delta_time);
    cast_rays(game->player);
}

void render(game_t* game)
{
    clear_colour_buf(0xFF000000);
    render_wall_projection(game->player);

    // Render minimap
    // render_map();
    // render_rays(game->player);
    // render_player(&(game->player));

    render_colour_buf();
}

void cleanup(void)
{
    free_wall_textures();
    free_graphics();
}
