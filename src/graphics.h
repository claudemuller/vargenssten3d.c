#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

bool init_window(const char *win_title);
void clear_colour_buf(const uint32_t colour);
void render_colour_buf(void);
void draw_pixel(const int x, const int y, const uint32_t colour);
void draw_line(const int x0, const int y0, const int x1, const int y1, const uint32_t colour);
void draw_rect(const int x, const int y, const int w, const int h, const uint32_t colour);
void free_graphics(void);

#endif // GRAPHICS_H
