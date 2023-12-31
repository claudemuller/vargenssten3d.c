#ifndef CONSTANTS_H
#define CONSTANTS_H

#define FPS 30
#define FRAME_TIME_LEN (1000.0 / FPS)

#define PI 3.14159265
#define TWO_PI 6.28318530

#define FOV_ANGLE (60 * PI / 180)
#define DISTANCE_PROJECTION_PLANE ((WINDOW_WIDTH / 2.0) / tan(FOV_ANGLE / 2))

#define TILE_SIZE 64

#endif // CONSTANTS_H
