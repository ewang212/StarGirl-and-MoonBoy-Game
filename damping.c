#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// window constants
const vector_t WINDOW_MIN = {.x = 0, .y = 0};
const vector_t WINDOW_MAX = {.x = 1000, .y = 500};
const size_t Y_HALF = 250;

// spring force constants
const size_t SPRING = 20;
const size_t DRAG = 5;

// circle constants
const float PI = M_PI;
const size_t HALF_CIRCLE = 180;

const double CIRCLE_RADIUS = 5;
const vector_t START_CIRCLE_POS = {.x = 15, .y = 250};

const size_t NUM_CIRCLES = 100;
const size_t CIRCLE_POINTS = 360;
const double CIRCLE_MASS = 20.0;

// color constants
const double COLOR_BOUNDARY = 0.6;
const double COLOR_REDUCTION = 0.2;
const rgb_color_t COLOR_WHITE = {1.0, 1.0, 1.0};

// line constants
const double LINE_DEN = 1.5;

typedef struct state {
  scene_t *circle_list;
  scene_t *parallel_list;
  float time;
} state_t;

body_t *make_circle(size_t center_x, size_t center_y, rgb_color_t color,
                    double mass) {
  list_t *circle_shape = list_init(CIRCLE_POINTS, free);
  assert(circle_shape != NULL);

  // make circle
  for (size_t i = 1; i < CIRCLE_POINTS; i++) {
    double curr_angle = (i * PI) / HALF_CIRCLE;
    size_t x = cos(curr_angle) * CIRCLE_RADIUS + center_x;
    size_t y = sin(curr_angle) * CIRCLE_RADIUS + center_y;
    vector_t *pnt = malloc(sizeof(vector_t));
    *pnt = (vector_t){.x = x, .y = y};
    list_add(circle_shape, pnt);
  }

  body_t *circle = body_init(circle_shape, mass, color);
  body_set_centroid(circle, (vector_t){.x = center_x, .y = center_y});
  return circle;
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->circle_list = scene_init();
  state->parallel_list = scene_init();
  state->time = 0.0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);

  // create circles
  vector_t center = START_CIRCLE_POS;

  double amplitude = (WINDOW_MAX.y - CIRCLE_RADIUS) / LINE_DEN;

  rgb_color_t color = COLOR_WHITE;

  for (size_t i = 0; i < NUM_CIRCLES; i++) {
    body_t *circle = make_circle(
        center.x, amplitude * sin(center.x * (PI / HALF_CIRCLE)) + WINDOW_MAX.y,
        color, CIRCLE_MASS);
    body_t *parallel = make_circle(center.x, Y_HALF, color, CIRCLE_MASS);
    scene_add_body(state->circle_list, circle);
    scene_add_body(state->parallel_list, parallel);

    // update values for next circle
    center.x = center.x + (2 * CIRCLE_RADIUS);
    // changing color
    if (color.g < COLOR_BOUNDARY) {
      color = COLOR_WHITE;
    } else if (color.b < COLOR_BOUNDARY) {
      color.g = color.g - COLOR_REDUCTION;
    } else if (color.r < COLOR_BOUNDARY) {
      color.b = color.b - COLOR_REDUCTION;
    } else {
      color.r = color.r - COLOR_REDUCTION;
    }
  }

  // apply forces
  for (size_t i = 0; i < NUM_CIRCLES; i++) {
    create_spring(state->circle_list, SPRING,
                  scene_get_body(state->circle_list, i),
                  scene_get_body(state->parallel_list, i));
    create_drag(state->circle_list, DRAG,
                scene_get_body(state->circle_list, i));
  }
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  // find time
  double dt = time_since_last_tick();
  state->time = state->time + dt;

  scene_tick(state->circle_list, dt);
  if (!sdl_is_done(state)) {
    for (size_t i = 0; i < scene_bodies(state->circle_list); i++) {
      sdl_draw_polygon(body_get_vertices(scene_get_body(state->circle_list, i)),
                       body_get_color(scene_get_body(state->circle_list, i)));
    }
    sdl_show();
  }
}

// frees the state
void emscripten_free(state_t *state) {
  scene_free(state->circle_list);
  free(state);
}
