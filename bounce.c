#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// window constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t CENTER = {.x = 500, .y = 250};
const size_t ROTATE = 10;

// star constants
const size_t NUM_CIRC_POINTS = 10;
const size_t STAR_RAD = 100;
const size_t SPEED = 400;

// color constants
const float YELLOW_R = 1.0;
const float YELLOW_G = 0.5;
const float YELLOW_B = 0.5;

const double TWO_PI = 2.0 * M_PI;

// returning list of vertices of star (counter-clockwise)
list_t *make_star(size_t radius, size_t num_points, size_t center_x,
                  size_t center_y) {
  double x;
  double y;
  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);
  for (size_t i = 0; i <= num_points - 1; i += 2) {

    // outside points
    x = radius * cos((TWO_PI * (i / 2)) / (num_points / 2)) + center_x;
    y = radius * sin((TWO_PI * (i / 2)) / (num_points / 2)) + center_y;

    vector_t *temp = malloc(sizeof(vector_t));
    *temp = (vector_t){x, y};
    list_add(vertices, temp);

    // inside points
    x = (radius / 2) * cos(TWO_PI * (i + 1) / num_points) + center_x;
    y = (radius / 2) * sin(TWO_PI * (i + 1) / num_points) + center_y;

    vector_t *tem = malloc(sizeof(vector_t));
    *tem = (vector_t){x, y};
    list_add(vertices, tem);
  }
  return vertices;
}

// state definition
typedef struct state {
  list_t *star;
  vector_t *velocity;
} state_t;

// initializes the state to hold the star polygon and returns the state
state_t *emscripten_init() {
  vector_t min = {.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->star = make_star(STAR_RAD, NUM_CIRC_POINTS, CENTER.x, CENTER.y);
  state->velocity = malloc(sizeof(vector_t));
  *state->velocity = (vector_t){SPEED, SPEED};
  return state;
}

// checks which boundary it bumps into
void boundary(state_t *state, list_t *star) {
  for (size_t i = 0; i < NUM_CIRC_POINTS; i++) {
    if (((vector_t *)list_get(star, i))->x < 0) {
      // bounce to right
      state->velocity->x = state->velocity->x * -1;
      if (((vector_t *)list_get(star, i))->x < 0) {
        vector_t dist = {.x = 0 - ((vector_t *)list_get(star, i))->x, .y = 0};
        polygon_translate(star, dist);
      }
    }
    if (((vector_t *)list_get(star, i))->x > WINDOW.x) {
      // bounce to left
      state->velocity->x = state->velocity->x * -1;
      if (((vector_t *)list_get(star, i))->x > WINDOW.x) {
        vector_t dist = {.x = WINDOW.x - ((vector_t *)list_get(star, i))->x,
                         .y = 0};
        polygon_translate(star, dist);
      }
    }
    if (((vector_t *)list_get(star, i))->y < 0) {
      // bounce up
      state->velocity->y = state->velocity->y * -1;
      if (((vector_t *)list_get(star, i))->y < 0) {
        vector_t dist = {.x = 0, .y = 0 - ((vector_t *)list_get(star, i))->y};
        polygon_translate(star, dist);
      }
    }
    if (((vector_t *)list_get(star, i))->y > WINDOW.y) {
      // bounce down
      state->velocity->y = state->velocity->y * -1;
      if (((vector_t *)list_get(star, i))->y > WINDOW.y) {
        vector_t dist = {.x = 0,
                         .y = WINDOW.y - ((vector_t *)list_get(star, i))->y};
        polygon_translate(star, dist);
      }
    }
  }
}
// translates and rotates the star
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  list_t *star = state->star;

  rgb_color_t yellow = {.r = YELLOW_R, .g = YELLOW_G, .b = YELLOW_B};
  sdl_draw_polygon(star, yellow);
  sdl_show();
  vector_t trans = vec_multiply(dt, *state->velocity);
  polygon_translate(star, trans);
  vector_t rot = polygon_centroid(star);
  polygon_rotate(star, ROTATE, rot);
  boundary(state, star);
  // checks boundary domains
}

// frees the memory associated with everything
void emscripten_free(state_t *state) {
  list_t *star = state->star;
  vector_t *velocity = state->velocity;
  free(star);
  free(velocity);
  free(state);
}
