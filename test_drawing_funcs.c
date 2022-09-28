/*
 * Test cases for drawing functions and associated helper functions for drawing functions program
 * CSF Assignment 2
 * Iris Gupta and Eric Wang
 * igupta5@jh.edu and ewang42@jhu.edu
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "drawing_funcs.h"
#include "tctest.h"

// an expected color identified by a (non-zero) character code
typedef struct {
  char c;
  uint32_t color;
} ExpectedColor;

// struct representing a "picture" of an expected image
typedef struct {
  ExpectedColor colors[20];
  const char *pic;
} Picture;

typedef struct {
  struct Image small;
  struct Image large;
  struct Image tilemap;
  struct Image spritemap;
} TestObjs;

// dimensions and pixel index computation for "small" test image (objs->small)
#define SMALL_W        8
#define SMALL_H        6
#define SMALL_IDX(x,y) ((y)*SMALL_W + (x))

// dimensions of the "large" test image
#define LARGE_W        24
#define LARGE_H        20

// create test fixture data
TestObjs *setup(void) {
  TestObjs *objs = (TestObjs *) malloc(sizeof(TestObjs));
  init_image(&objs->small, SMALL_W, SMALL_H);
  init_image(&objs->large, LARGE_W, LARGE_H);
  objs->tilemap.data = NULL;
  objs->spritemap.data = NULL;
  return objs;
}

// clean up test fixture data
void cleanup(TestObjs *objs) {
  free(objs->small.data);
  free(objs->large.data);
  free(objs->tilemap.data);
  free(objs->spritemap.data);

  free(objs);
}

uint32_t lookup_color(char c, const ExpectedColor *colors) {
  for (unsigned i = 0; ; i++) {
    assert(colors[i].c != 0);
    if (colors[i].c == c) {
      return colors[i].color;
    }
  }
}

void check_picture(struct Image *img, Picture *p) {
  unsigned num_pixels = img->width * img->height;
  assert(strlen(p->pic) == num_pixels);

  for (unsigned i = 0; i < num_pixels; i++) {
    char c = p->pic[i];
    uint32_t expected_color = lookup_color(c, p->colors);
    uint32_t actual_color = img->data[i];
    ASSERT(actual_color == expected_color);
  }
}

// prototypes of test main functions
void test_draw_pixel(TestObjs *objs);
void test_draw_rect(TestObjs *objs);
void test_draw_circle(TestObjs *objs);
void test_draw_circle_clip(TestObjs *objs);
void test_draw_tile(TestObjs *objs);
void test_draw_sprite(TestObjs *objs);
// prototypes of test helper functions
void test_in_bounds(TestObjs *objs);
void test_compute_index(TestObjs *objs); 
void test_clamp(TestObjs *objs);
void test_get_r(TestObjs *objs);
void test_get_g(TestObjs *objs);
void test_get_b(TestObjs *objs);
void test_get_a(TestObjs *objs);
void test_blend_components(TestObjs *objs);
void test_blend_colors(TestObjs *objs);
void test_set_pixel(TestObjs *objs);
void test_square(TestObjs *objs);
void test_square_dist(TestObjs *objs);


int main(int argc, char **argv) {
  if (argc > 1) {
    // user specified a specific test function to run
    tctest_testname_to_execute = argv[1];
  }

  TEST_INIT();

  // TEST() directives for main functions
  TEST(test_draw_pixel);
  TEST(test_draw_rect);
  TEST(test_draw_circle);
  TEST(test_draw_circle_clip);
  TEST(test_draw_tile);
  TEST(test_draw_sprite);
  // TEST() directives for helper functions
  TEST(test_in_bounds);
  TEST(test_compute_index);
  TEST(test_clamp);
  TEST(test_get_r);
  TEST(test_get_g);
  TEST(test_get_b);
  TEST(test_get_a);
  TEST(test_blend_components);
  TEST(test_blend_colors);
  TEST(test_set_pixel);
  TEST(test_square);
  TEST(test_square_dist);
  
  
  TEST_FINI();
}

void test_draw_pixel(TestObjs *objs) {
  // initially objs->small pixels are opaque black
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x000000FFU);
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x000000FFU);

  // test drawing completely opaque pixels
  draw_pixel(&objs->small, 3, 2, 0xFF0000FF); // opaque red
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0xFF0000FF);
  draw_pixel(&objs->small, 5, 4, 0x800080FF); // opaque magenta (half-intensity)
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x800080FF);

  // test color blending
  draw_pixel(&objs->small, 3, 2, 0x00FF0080); // half-opaque full-intensity green
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x7F8000FF);
  draw_pixel(&objs->small, 4, 2, 0x0000FF40); // 1/4-opaque full-intensity blue
  ASSERT(objs->small.data[SMALL_IDX(4, 2)] == 0x000040FF);
}

void test_draw_rect(TestObjs *objs) {
  struct Rect red_rect = { .x = 2, .y = 2, .width=3, .height=3 };
  struct Rect blue_rect = { .x = 3, .y = 3, .width=3, .height=3 };
  draw_rect(&objs->small, &red_rect, 0xFF0000FF); // red is full-intensity, full opacity
  draw_rect(&objs->small, &blue_rect, 0x0000FF80); // blue is full-intensity, half opacity

  const uint32_t red   = 0xFF0000FF; // expected full red color
  const uint32_t blue  = 0x000080FF; // expected full blue color
  const uint32_t blend = 0x7F0080FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "        "
    "        "
    "  rrr   "
    "  rnnb  "
    "  rnnb  "
    "   bbb  "
  };

  check_picture(&objs->small, &expected);
}

void test_draw_circle(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    "   x    "
    "  xxx   "
    " xxxxx  "
    "  xxx   "
    "   x    "
    "        "
  };

  draw_circle(&objs->small, 3, 2, 2, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_circle_clip(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    " xxxxxxx"
    " xxxxxxx"
    "xxxxxxxx"
    " xxxxxxx"
    " xxxxxxx"
    "  xxxxx "
  };

  draw_circle(&objs->small, 4, 2, 4, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_tile(TestObjs *objs) {
  ASSERT(read_image("img/PrtMimi.png", &objs->tilemap) == IMG_SUCCESS);

  struct Rect r = { .x = 4, .y = 2, .width = 16, .height = 18 };
  draw_rect(&objs->large, &r, 0x1020D0FF);

  struct Rect grass = { .x = 0, .y = 16, .width = 16, .height = 16 };
  draw_tile(&objs->large, 3, 2, &objs->tilemap, &grass);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x52ad52ff },
      { 'b', 0x1020d0ff },
      { 'c', 0x257b4aff },
      { 'd', 0x0c523aff },
    },
    "                        "
    "                        "
    "             a     b    "
    "            a      b    "
    "            a     ab    "
    "           ac      b    "
    "           ac a    b    "
    "      a a  ad  a   b    "
    "     a  a aad  aa ab    "
    "     a  a acd aaacab    "
    "    aa  cdacdaddaadb    "
    "     aa cdaddaaddadb    "
    "     da ccaddcaddadb    "
    "    adcaacdaddddcadb    "
    "   aaccacadcaddccaab    "
    "   aacdacddcaadcaaab    "
    "   aaaddddaddaccaacb    "
    "   aaacddcaadacaaadb    "
    "    bbbbbbbbbbbbbbbb    "
    "    bbbbbbbbbbbbbbbb    "
  };

  check_picture(&objs->large, &pic);
}

void test_draw_sprite(TestObjs *objs) {
  ASSERT(read_image("img/NpcGuest.png", &objs->spritemap) == IMG_SUCCESS);

  struct Rect r = { .x = 1, .y = 1, .width = 14, .height = 14 };
  draw_rect(&objs->large, &r, 0x800080FF);

  struct Rect sue = { .x = 128, .y = 136, .width = 16, .height = 15 };
  draw_sprite(&objs->large, 4, 2, &objs->spritemap, &sue);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x800080ff },
      { 'b', 0x9cadc1ff },
      { 'c', 0xefeae2ff },
      { 'd', 0x100000ff },
      { 'e', 0x264c80ff },
      { 'f', 0x314e90ff },
    },
    "                        "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaabccccccbc       "
    " aaaaacccccccccccc      "
    " aaaacbddcccddcbccc     "
    " aaacbbbeccccedbccc     "
    " aaacbbceccccebbbccc    "
    " aaabbbccccccccbbccc    "
    " aaaabbbcccccccb ccb    "
    " aaaabaaaaabbaa  cb     "
    " aaaaaaaaafffea         "
    " aaaaaaaaaffeea         "
    " aaaaaaacffffcc         "
    "        cffffccb        "
    "         bbbbbbb        "
    "                        "
    "                        "
    "                        "
  };

  check_picture(&objs->large, &pic);
}

void test_in_bounds(TestObjs *objs) {
  ASSERT(in_bounds(&objs->small, 0, 0) == 0);
  ASSERT(in_bounds(&objs->small, -1, 0) == 1);
  ASSERT(in_bounds(&objs->small, 0, -1) == 1);
  ASSERT(in_bounds(&objs->small, -1, -1) == 1);
  
  ASSERT(in_bounds(&objs->small, 7, 5) == 0);
  ASSERT(in_bounds(&objs->small, 8, 5) == 1);
  ASSERT(in_bounds(&objs->small, 7, 6) == 1);
  ASSERT(in_bounds(&objs->small, 8, 6) == 1);
}

void test_compute_index(TestObjs *objs) {
  ASSERT(compute_index(&objs->small, 0, 0) == 0);
  ASSERT(compute_index(&objs->small, 1, 0) == 1);
  ASSERT(compute_index(&objs->small, 0, 1) == 8);
  ASSERT(compute_index(&objs->small, 1, 1) == 9);
  ASSERT(compute_index(&objs->small, 7, 5) == 47);
}

void test_clamp(TestObjs *objs) {
  (void) objs;
  ASSERT(clamp(50, 10, 100) == 50);
  ASSERT(clamp(0, 10, 100) == 10);
  ASSERT(clamp(1000, 10, 100) == 100);
  ASSERT(clamp(10, 10, 100) == 10);
  ASSERT(clamp(100, 10, 100) == 100);
  ASSERT(clamp(-10, 0, 100) == 0);
}

void test_get_r(TestObjs *objs) {
  (void) objs;
  const uint32_t red = 0xFF0000FF;                    
  const uint32_t blue = 0x000080FF;                      
  const uint32_t blend = 0x7F0080FF;                 
  const uint32_t black = 0x000000FF;
  
  ASSERT(get_r(red) == 0xFF);
  ASSERT(get_r(blue) == 0x00);
  ASSERT(get_r(blend) == 0x7F);
  ASSERT(get_r(black) == 0x00);
}

void test_get_g(TestObjs *objs) {
  (void) objs;
  const uint32_t red = 0xFF0000FF;
  const uint32_t blue = 0x000080FF;
  const uint32_t black = 0x000000FF;
  const uint32_t new1 = 0x001100FF;
  const uint32_t new2 = 0x00FF00FF;
  
  ASSERT(get_g(red) == 0x00);
  ASSERT(get_g(blue) == 0x00);
  ASSERT(get_g(black) == 0x00);
  ASSERT(get_g(new1) == 0x11);
  ASSERT(get_g(new2) == 0xFF);
}

void test_get_b(TestObjs *objs) {
  (void) objs;
  const uint32_t red = 0xFF0000FF;
  const uint32_t blue = 0x000080FF;
  const uint32_t black = 0x000000FF;
  const uint32_t new1 = 0x000011FF;
  const uint32_t new2 = 0x0000FFFF;

  ASSERT(get_b(red) == 0x00);
  ASSERT(get_b(blue) == 0x80);
  ASSERT(get_b(black) == 0x00);
  ASSERT(get_b(new1) == 0x11);
  ASSERT(get_b(new2) == 0xFF);  
}

void test_get_a(TestObjs *objs) {
  (void) objs;
  const uint32_t red = 0xFF0000FF;
  const uint32_t blue = 0x000080FF;
  const uint32_t black = 0x000000FF;
  const uint32_t new1 = 0x0000FF00;
  const uint32_t new2 = 0x0000FF11;
  
  ASSERT(get_a(red) == 0xFF);
  ASSERT(get_a(blue) == 0xFF);
  ASSERT(get_a(black) == 0xFF);
  ASSERT(get_a(new1) == 0x00);
  ASSERT(get_a(new2) == 0x11);
}

void test_blend_components(TestObjs *objs) {
  (void) objs;
  const uint32_t red = 0xFF0000FF;
  const uint32_t blue = 0x000080FF;
  
  uint32_t r_val = blend_components(0, 255, 255);
  uint32_t g_val = blend_components(0, 0, 255);
  uint32_t b_val = blend_components(get_b(blue), get_b(red), get_a(red));
  ASSERT(r_val == 0x00);
  ASSERT(g_val == 0x00);
  ASSERT(b_val == 0x80);

  r_val = blend_components(get_r(red), get_r(blue), get_a(red));
  g_val = blend_components(get_g(red), get_g(blue), get_a(red));
  b_val = blend_components(get_b(red), get_b(blue), get_a(red));
  ASSERT(r_val == 0xFF);
  ASSERT(g_val == 0x00);
  ASSERT(b_val == 0x00);
}

void test_blend_colors(TestObjs *objs) {
  (void) objs;
  const uint32_t red   = 0xFF0000FF;                     
  const uint32_t blue  = 0x000080FF;                      
  const uint32_t blend = 0x7F0080FF;                 
  const uint32_t black = 0x000000FF;
  
  ASSERT(blend_colors(red, blue) == 0xFF0000FF);
  ASSERT(blend_colors(blue, red) == 0x000080FF);
  ASSERT(blend_colors(red, black) == 0xFF0000FF);
  ASSERT(blend_colors(blue, black) == 0x000080FF);
  ASSERT(blend_colors(red, blend) == 0xFF0000FF);
  ASSERT(blend_colors(blue, blend) == 0x000080FF);
}

void test_set_pixel(TestObjs *objs) {
  uint32_t index1 = compute_index(&objs->small, 3, 2);
  uint32_t index2 = compute_index(&objs->small, 5, 4);
  uint32_t index3 = compute_index(&objs->small, 4, 2);
  
  // initially objs->small pixels are opaque black
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x000000FFU);
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x000000FFU);
  
  // test setting completely opaque pixels
  set_pixel(&objs->small, index1, 0xFF0000FF); // opaque red
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0xFF0000FF);
  set_pixel(&objs->small, index2, 0x800080FF); // opaque magenta (half-intensity)
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x800080FF);
  
  // test color blending
  set_pixel(&objs->small, index1, 0x00FF0080); // half-opaque full-intensity green
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x7F8000FF);
  set_pixel(&objs->small, index3, 0x0000FF40); // 1/4-opaque full-intensity blue
  ASSERT(objs->small.data[SMALL_IDX(4, 2)] == 0x000040FF);
}

void test_square(TestObjs *objs) {
  (void) objs;
  int64_t a = 0;
  int64_t b = 1;
  int64_t c = 0xFFFF;

  ASSERT(square(a) == 0);
  ASSERT(square(a) == a);
  ASSERT(square(b) == 1);
  ASSERT(square(b) == b);
  ASSERT(square(c) == 0xFFFE0001);
}

void test_square_dist(TestObjs *objs) {
  (void) objs;
  ASSERT(square_dist(0, 0, 3, 4) == 25);
  ASSERT(square_dist(3, 4, 0, 0) == 25);
  ASSERT(square_dist(-3, -4, 0, 0) == 25);
  ASSERT(square_dist(3, 0, 0, 4) == 25);
  ASSERT(square_dist(0, 4, 3, 0) == 25);
}
