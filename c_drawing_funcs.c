// C implementations of drawing functions (and helper functions)

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "drawing_funcs.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////

static int32_t in_bounds(struct Image *img, int32_t x, int32_t y) {
  if ((x >= img->width) || (y >= img->height) || x<0 || y<0) {
    return 1;
  }
  return 0;
}

uint32_t compute_index(struct Image *img, int32_t x, int32_t y) {
  uint32_t val = y * (img->width) + x;
  return val;
}

static int32_t clamp(int32_t val, int32_t min, int32_t max) {
  if (val < min) {
    val = min;
  }
  else if (val>max) {
    val = max;
  }
  return val;
}

uint8_t get_r(uint32_t color) {
  return (color >> 24) & 255;
}

uint8_t get_g(uint32_t color) {
  return (color >> 16) & 255;
}

uint8_t get_b(uint32_t color) {
  return (color >> 8) & 255;
}

uint8_t get_a(uint32_t color) {
  return color & 255;
}

static uint8_t blend_components(uint32_t fg, uint32_t bg, uint32_t alpha) {
  return (alpha*fg+(255-alpha)*bg)/255;
}

uint32_t blend_colors(uint32_t fg, uint32_t bg) {
  uint32_t r = blend_components(get_r(fg), get_r(bg), get_a(fg));
  uint32_t g = blend_components(get_g(fg), get_g(bg), get_a(fg));
  uint32_t b = blend_components(get_b(fg), get_b(bg), get_a(fg));
  uint32_t final = (r << 24) + (g << 16) + (b << 8) + 256;
  return final;
}

void set_pixel(struct Image *img, uint32_t index, uint32_t color) {
  uint32_t bg_color = img->data[index];
  img->data[index] = blend_colors(color, bg_color);
}

int64_t square(int64_t x) {
  int64_t val = x * x;
  return val;
}

int64_t square_dist(int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
  int64_t val = square(x1 - x2) + square(y1 - y2);
  return val;
}

////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////

//
// Draw a pixel.
//
// Parameters:
//   img   - pointer to struct Image
//   x     - x coordinate (pixel column)
//   y     - y coordinate (pixel row)
//   color - uint32_t color value
//
void draw_pixel(struct Image *img, int32_t x, int32_t y, uint32_t color) {
  if (in_bounds(img, x, y) == 1) {
    return;
  }
  uint32_t index = compute_index(img, x, y);
  set_pixel(img, index, color);
}

//
// Draw a rectangle.
// The rectangle has rect->x,rect->y as its upper left corner,
// is rect->width pixels wide, and rect->height pixels high.
//
// Parameters:
//   img     - pointer to struct Image
//   rect    - pointer to struct Rect
//   color   - uint32_t color value
//
void draw_rect(struct Image *img, const struct Rect *rect, uint32_t color) {
  int32_t min_x = clamp(rect->x, 0, img->width);
  int32_t max_x = clamp(rect->x + rect->width, 0, img->width);
  int32_t min_y = clamp(rect->y, 0, img->height);
  int32_t max_y = clamp(rect->y + rect->height, 0, img->height);
  for (int i = min_x; i < max_x; i++) {
    for (int j = min_y; j < max_y; j++) {
      draw_pixel(img, i, j, color);
    }
  }
}

//
// Draw a circle.
// The circle has x,y as its center and has r as its radius.
//
// Parameters:
//   img     - pointer to struct Image
//   x       - x coordinate of circle's center
//   y       - y coordinate of circle's center
//   r       - radius of circle
//   color   - uint32_t color value
//
void draw_circle(struct Image *img, int32_t x, int32_t y, int32_t r, uint32_t color) {
  for (int i = 0; i < img->height; i++) {
    for (int j = 0; j < img->width; j++) {
      if (square_dist(j, i, x, y) <= square(r)) {
        draw_pixel(img, j, i, color);
      }
    }
  }
}

//
// Draw a tile by copying all pixels in the region
// enclosed by the tile parameter in the tilemap image
// to the specified x/y coordinates of the destination image.
// No blending of the tile pixel colors with the background
// colors should be done.
//
// Parameters:
//   img     - pointer to Image (dest image)
//   x       - x coordinate of location where tile should be copied
//   y       - y coordinate of location where tile should be copied
//   tilemap - pointer to Image (the tilemap)
//   tile    - pointer to Rect (the tile)
//
void draw_tile(struct Image *img, int32_t x, int32_t y, struct Image *tilemap, const struct Rect *tile) {
  if (in_bounds(tilemap, tile->x, tile->y) == 1) {
    return;
  }
  if (in_bounds(tilemap, tile->x + tile->width, tile->y + tile->height) == 1) {
    return;
  }
  int32_t clampedWidth = clamp(tile->width, 0, img->width - x);
  int32_t clampedHeight = clamp(tile->height, 0, img->height - y);
  int32_t yIndex = y;
  int32_t xIndex = x;
  for (int i = tile->x; i < tile->x + clampedWidth; i++) {
    yIndex = y;
    for (int j = tile->y; j < tile->y + clampedHeight; j++) {
      int tileIndex = compute_index(tilemap, i, j);
      int imageIndex = compute_index(img, xIndex, yIndex);
      img->data[imageIndex] = tilemap->data[tileIndex];
      yIndex++;
    }
    xIndex++;
  }
}

//
// Draw a sprite by copying all pixels in the region
// enclosed by the sprite parameter in the spritemap image
// to the specified x/y coordinates of the destination image.
// The alpha values of the sprite pixels should be used to
// blend the sprite pixel colors with the background
// pixel colors.
//
// Parameters:
//   img       - pointer to Image (dest image)
//   x         - x coordinate of location where sprite should be copied
//   y         - y coordinate of location where sprite should be copied
//   spritemap - pointer to Image (the spritemap)
//   sprite    - pointer to Rect (the sprite)
//
void draw_sprite(struct Image *img, int32_t x, int32_t y, struct Image *spritemap, const struct Rect *sprite) {
  if (in_bounds(spritemap, sprite->x, sprite->y) == 1) {
    return;
  }
  if (in_bounds(spritemap, sprite->x + sprite->width, sprite->y + sprite->height) == 1) {
    return;
  }
  int32_t clampedWidth = clamp(sprite->width, 0, img->width - x);
  int32_t clampedHeight = clamp(sprite->height, 0, img->height - y);
  int32_t yIndex = y;
  int32_t xIndex = x;
  for (int i = tile->x; i < tile->x + clampedWidth; i++) {
    yIndex = y;
    for (int j = tile->y; j < tile->y + clampedHeight; j++) {
      int tileIndex = compute_index(spritemap, i, j);
      draw_pixel(img, xIndex, yIndex, spritemap->data[spriteIndex]);
      yIndex++;
    }
    xIndex++;
  }
}
