/*
 * Implementation of drawing functions and associated helper functions for drawing functions program
 * CSF Assignment 2
 * Iris Gupta and Eric Wang
 * igupta5@jh.edu and ewang42@jhu.edu
 */

// C implementations of drawing functions (and helper functions)

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "drawing_funcs.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////

/*
 * Determine whether or not a particular pixel (represented by an x/y position) falls within
 * the width and height range of an image (containing x/y positions)
 * If the pixel falls within the image range, 0 is returned
 * If the pixel is not within the image range, 1 is returned
 *
 * Parameters:
 *   img  - pointer to the image struct
 *   x - int32_t value of width position of pixel
 *   y - int32_t value of height position of pixel
 *
 * Returns:
 *   an integer (0 or 1) represented the status of an x and y posiition
 *   being within the bounds of an image or not
 */
static int32_t in_bounds(struct Image *img, int32_t x, int32_t y) {
  if ((x >= img->width) || (y >= img->height) || x<0 || y<0) {
    return 1;
  }
  return 0;
}

/*
 * Converts the x/y position representing a pixel within an image to an
 * index within an array pointed to by "data" within the image struct containing color information
 * returns the index within the array pointed to by "data" represented by the
 * equivalent x/y position given. This function is only ever called after 
 * in_bounds returns 0 and does not handle out of bounds x/y values.
 * 
 * Parameters:
 *   img  - pointer to the image struct
 *   x - int32_t value of width position of pixel
 *   y - int32_t value of height position of pixel
 *
 * Returns:
 *   a uint32_t value that represents the index within the array pointed to by "data"
 *   that would contain the same information as the (x,y) within the image
 */
uint32_t compute_index(struct Image *img, int32_t x, int32_t y) {
  uint32_t val = y * (img->width) + x;
  return val;
}

/*
 * Constrains a value within the bounds of (min, max) if it
 * is not already within range. if val is lower than the min,
 * it is set to the min. if val is higher than the max, it is
 * set to the max. if val is already between min and max, it is
 * left alone. returns the adjusted val value
 * 
 * Parameters:
 *   val - int32_t value to be constrained within (min, max)
 *   min - int32_t value of lower bound of constraint
 *   max - int32_t value of upper bound of constraint
 *
 * Returns:
 *   a int32_t value that is within the bounds of (min, max)
 */
static int32_t clamp(int32_t val, int32_t min, int32_t max) {
  if (val < min) {
    val = min;
  }
  else if (val>max) {
    val = max;
  }
  return val;
}

/*
 * parses the red value from a uint32_t color value by 
 * isolating the 8 left-most bits. returns the 8 left-most bits of 
 * the whole color value as the red value
 * 
 * Parameters:
 *   color - int32_t color value containing red, blue, green color information and the alpha information
 *
 * Returns:
 *   a uint8_t value that represents the red color value
 */
uint8_t get_r(uint32_t color) {
  return (color >> 24) & 255;
}

/*
 * parses the green value from a uint32_t color value by 
 * isolating the 8 middle-left bits. returns the 8 middle-left bits of 
 * the whole color value as the green value
 * 
 * Parameters:
 *   color - int32_t color value containing red, blue, green color information and the alpha information
 *
 * Returns:
 *   a uint8_t value that represents the green color value
 */
uint8_t get_g(uint32_t color) {
  return (color >> 16) & 255;
}

/*
 * parses the blue value from a uint32_t color value by 
 * isolating the 8 middle-right bits. returns the 8 middle-right bits of 
 * the whole color value as the blue value
 * 
 * Parameters:
 *   color - int32_t color value containing red, blue, green color information and the alpha information
 *
 * Returns:
 *   a uint8_t value that represents the blue color value
 */
uint8_t get_b(uint32_t color) {
  return (color >> 8) & 255;
}

/*
 * parses the alpha value from a uint32_t color value by 
 * isolating the 8 right-most bits. returns the 8 right-most bits of 
 * the whole color value as the alpha (opacity) value
 * 
 * Parameters:
 *   color - int32_t color value containing red, blue, green color information and the alpha information
 *
 * Returns:
 *   a uint8_t value that represents the alpha value
 */
uint8_t get_a(uint32_t color) {
  return color & 255;
}

/*
 * calculates the value of an individual color component
 * in a foreground image (either red, blue, or green) being blended
 * with the value of that same individual color component in a 
 * background image. returns the calculated color blended value.
 * 
 * Parameters:
 *   fg - uint32_t value representing a color component value in the foreground image
 *   bg - uint32_t value representing a color component value in the background image
 *   alpha - uint32_t value representing the alpha value to be used in blending calculation
 *
 * Returns:
 *   a uint8_t value that represents the blended color component value
 */
static uint8_t blend_components(uint32_t fg, uint32_t bg, uint32_t alpha) {
  return (alpha*fg+(255 - alpha)*bg)/255;
}

/*
 * compile the blended values for each color component into a wholistic color value
 * by building a 32-bit integer out of 8-bit color component values.
 * sets alpha value to 255 for total opacity. returns this final color value
 * 
 * Parameters:
 *   fg - uint32_t value representing a color value in the foreground image
 *   bg - uint32_t value representing a color value in the background image
 *
 * Returns:
 *   a uint32_t value that represents the blended color component value
 */
uint32_t blend_colors(uint32_t fg, uint32_t bg) {
  uint32_t r = blend_components(get_r(fg), get_r(bg), get_a(fg));
  uint32_t g = blend_components(get_g(fg), get_g(bg), get_a(fg));
  uint32_t b = blend_components(get_b(fg), get_b(bg), get_a(fg));
  uint32_t final = (r << 24) + (g << 16) + (b << 8) + 255;
  return final;
}

/*
 * modify a pixel in the background image to represent the blended
 * color value of the corresponding pixels in the background and foreground images
 * 
 * Parameters:
 *   img - pointer to the struct Image
 *   index - uint32_t value representing the index in the background image color array whose value should be modified
 *   color - uint32_t value representing a color value in the background image
 */
void set_pixel(struct Image *img, uint32_t index, uint32_t color) {
  uint32_t bg_color = img->data[index];
  img->data[index] = blend_colors(color, bg_color);
}

/*
 * square a value
 * 
 * Parameters:
 *   x - int64_t value to be squared
 * 
 * Returns:
 *   a int64_t value that equals the square of x
 */
int64_t square(int64_t x) {
  int64_t val = x * x;
  return val;
}

/*
 * calculates the squared distance between two x coordinates and two y coordinates and 
 * adds them
 * 
 * Parameters:
 *   x1 - int64_t value representing an x coordinate
 *   x2 - int64_t value representing an x coordinate
 *   y1 - int64_t value representing an y coordinate
 *   y2 - int64_t value representing an y coordinate
 * 
 * Returns:
 *   a int64_t value that equals the sum of squared distances
 */
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
  for (int i = tile->x; i < tile->x + clampedWidth-1; i++) {
    yIndex = y;
    for (int j = tile->y; j < tile->y + clampedHeight-1; j++) {
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
  if (in_bounds(spritemap, sprite->x, sprite->y)==1) {
    return;
  }
  if (in_bounds(spritemap, sprite->x + sprite->width, sprite->y + sprite->height) == 1) {
    return;
  }
  int32_t clampedWidth = clamp(sprite->width, 0, img->width - x);
  int32_t clampedHeight = clamp(sprite->height, 0, img->height - y);
  int32_t yIndex = y;
  int32_t xIndex = x;
  for (int i = sprite->x; i < sprite->x + clampedWidth-1; i++) {
    yIndex = y;
    for (int j = sprite->y; j < sprite->y + clampedHeight-1; j++) {
      int spriteIndex = compute_index(spritemap, i, j);
      draw_pixel(img, xIndex, yIndex, spritemap->data[spriteIndex]);
      yIndex++;
    }
    xIndex++;
  }
}
