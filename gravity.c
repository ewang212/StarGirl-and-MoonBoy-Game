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

// window constant
const vector_t WINDOW = {.x = 1000, .y = 500};
const size_t OUTER_EDGE = 1200;

// polygon constant
const size_t STAR_RAD = 100;
const size_t START_X = 100;
const size_t START_Y = 400;
const size_t HIGH_POINTS = 8;
const size_t LOW_POINTS = 4;

// state definition
typedef struct state {
  list_t *polygon_list;
  float time;
  float count;
} state_t;

// initializes the list of polygon structure as well as the window
state_t *emscripten_init() {
  // initializes window
  vector_t min = {.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  // initializes state_t struct
  state_t *state = malloc(sizeof(state_t));
  list_t *list = list_init(WINDOW.x, (void *)polygon_free);
  size_t num_points = (rand() % (HIGH_POINTS) + LOW_POINTS) * 2;
  polygon_t *polygon =
      polygon_init(polygon_make_star_p(STAR_RAD, num_points, START_X, START_Y));
  list_add(list, (void *)polygon);
  state->count = 0.0;
  state->polygon_list = list;
  state->time = 0.0;
  return state;
}

// draws and creates new polygons as needed. Is called many times.
void emscripten_main(state_t *state) {
  // clears screen
  sdl_clear();
  // tracks time
  double dt = time_since_last_tick();
  state->time = state->time + dt;
  state->count = state->count + dt;

  // adds new polygon to list
  if (state->time > 1) {
    size_t num_points = (rand() % (HIGH_POINTS) + LOW_POINTS) * 2;
    polygon_t *polygon = polygon_init(
        polygon_make_star_p(STAR_RAD, num_points, START_X, START_Y));
    list_add(state->polygon_list, (void *)polygon);
    state->time = 0.0;
  }

  // moves polygon, redraws polygon on screen
  for (size_t i = 0; i < list_size(state->polygon_list); i++) {
    polygon_star_movement((polygon_t *)list_get(state->polygon_list, i),
                          state->time);
    sdl_draw_polygon(
        polygon_get_vertices((polygon_t *)(list_get(state->polygon_list, i))),
        polygon_get_color((polygon_t *)list_get(state->polygon_list, i)));
    sdl_show();
    // removes polygons that have exited screen
    if (polygon_centroid(polygon_get_vertices(
                             (polygon_t *)(list_get(state->polygon_list, i))))
            .x > OUTER_EDGE) {
      list_remove(state->polygon_list, i);
      i--;
    }
  }
}

// frees memory in the state_t struct
void emscripten_free(state_t *state) {
  for (size_t i = 0; i < list_size(state->polygon_list); i++) {
    polygon_free((polygon_t *)list_get(state->polygon_list, i));
  }
}
